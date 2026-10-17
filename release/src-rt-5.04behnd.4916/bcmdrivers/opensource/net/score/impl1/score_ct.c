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

#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/tcp.h>
#include <linux/bcm_nf_conntrack.h>
#include <linux/bcm_skb_defines.h>

#include "score_int.h"

typedef struct {
    stream_id_key_t id;
    uint16_t dcp_bmap;
    struct {
        uint32_t in_use:1;
        uint32_t rsvd:32;
    };
    struct nf_conn *ct;/* ptr to conntrack */
    void *dcp_priv[SCORE_MAX_DCP_NUM];/* dcp private data*/
} score_stream_id_obj_t;

typedef struct {
    spinlock_t lock;
    unsigned long free_idx_bitmap[STREAM_ID_AVAIL_BITMAP_NUM_WORDS];/* bitmap for availability */
    score_stream_id_obj_t entry[STREAM_ID_MAX_ENTRIES];
}score_stream_id_tbl_t;

/* for now static allocation should be ok, later if needed
 * we will just allocate tbl of pointers or use hash and 
 * dynamicllay allocate obj's
 */
static score_stream_id_tbl_t stream_id_tbl_g;
static score_stream_id_tbl_t *stream_id_tbl_p;

int score_streamid_destroy_on_ct_destroy=1;

#define STREAM_ID_TBL_LOCK()         spin_lock_bh(&stream_id_tbl_g.lock)
#define STREAM_ID_TBL_UNLOCK()       spin_unlock_bh(&stream_id_tbl_g.lock)

static inline int get_idx(uint16_t *idx_p)
{
    static int offset = 1; /* try to allocate sequentially */
    uint16_t idx = 0;
    volatile unsigned long *addr;
    addr = &stream_id_tbl_p->free_idx_bitmap[0];

    idx = find_next_bit((const unsigned long *)addr, STREAM_ID_MAX_ENTRIES, offset);
    if((idx == STREAM_ID_MAX_ENTRIES) && (offset !=1))
    {
        /* if entry not found and we did not start from beginning(offset 1)
         * do a second lookup, to find any free entires from 1 - offset 
         */
        offset = 1;
        idx = find_next_bit((const unsigned long *)addr, STREAM_ID_MAX_ENTRIES, offset);
    }

    if ((idx > 0) && (idx < STREAM_ID_MAX_ENTRIES))
    {
        *idx_p = idx;
        clear_bit(*idx_p, addr);

        /* update offset for next allocation */
        if (idx == (STREAM_ID_MAX_ENTRIES - 1))
            offset = 1;
        else
            offset = idx + 1;

        return 0;
    }
    return -1;
}

static void put_idx(uint16_t idx)
{
    volatile unsigned long *addr;
    if (idx < STREAM_ID_MAX_ENTRIES)
    {
        addr = &stream_id_tbl_p->free_idx_bitmap[0];
        set_bit(idx, addr);
    }
}

/*
 * Allocates an entry in score_entry_id_tbl_t 
 */
static inline uint16_t alloc_streamid(score_stream_id_obj_t **stream_obj_pp)
{
    uint16_t idx = 0;
    if (get_idx(&idx))
    {
        __logDebug(" streamid allocation Failed\n");
        return SCORE_STREAM_ID_INVALID;
    }
    *stream_obj_pp = &stream_id_tbl_p->entry[idx];
    (*stream_obj_pp)->in_use = 1;
    (*stream_obj_pp)->dcp_bmap = 0;
    __logDebug("Assigning idx[%u:%u] \n", (*stream_obj_pp)->id.incarn, (*stream_obj_pp)->id.idx);
    return (*stream_obj_pp)->id.key;
}

static inline score_stream_id_obj_t *streamid_to_streamobj(uint16_t streamkey)
{
    score_stream_id_obj_t *stream_obj;
    stream_id_key_t stream_id;
    stream_id.key = streamkey;

    if (unlikely((stream_id.idx == 0)))
    {
        __logNotice("invalid idx[%u] for streamid mapping\n", stream_id.idx);
        return NULL;
    }
    stream_obj = &stream_id_tbl_p->entry[stream_id.idx];
    if (unlikely(streamkey != stream_obj->id.key))
    {
        __logInfo("streamid[%u:%u] doesn't match with streamid in table[%u:%u]\n",
                stream_id.incarn, stream_id.idx, stream_obj->id.incarn, stream_obj->id.idx);
        return NULL;
    }
    return stream_obj;
}

/*
 *  Free the entry associated to the streamid 
 */
static inline void free_streamid(uint16_t streamkey)
{
    score_stream_id_obj_t *stream_obj;

    stream_obj = streamid_to_streamobj(streamkey);

    if(!stream_obj) {
        __logNotice("failed to find streamid mapping [%u]\n", streamkey);
        return;
    }

    __logDebug("Freeing idx[%u:%u] \n", stream_obj->id.incarn, stream_obj->id.idx);

    stream_obj->id.incarn++;
    /*this incarn will wrap around to 0 after max value represnt by incarn bits */
    if (stream_obj->id.incarn == 0)
        stream_obj->id.incarn = 1;

    stream_obj->ct = NULL;
    stream_obj->in_use = 0;
    put_idx(stream_obj->id.idx);
}

/*
 * Initializes the stream_id_tbl
 */
int stream_id_tbl_init(void)
{
    int idx;
    stream_id_tbl_p = &stream_id_tbl_g;
    memset(stream_id_tbl_p, 0, sizeof(score_stream_id_tbl_t));
    spin_lock_init(&stream_id_tbl_p->lock);
    for (idx=0; idx < STREAM_ID_MAX_ENTRIES; idx++)
    {
        stream_id_tbl_p->entry[idx].id.idx = idx;
        stream_id_tbl_p->entry[idx].id.incarn = 1;
        /*Mark this entry to be available for mapping */
        set_bit(idx,&stream_id_tbl_p->free_idx_bitmap[0]);
    }
    /* Mark the very first index to make id 0 as non-available */
    clear_bit(0,&stream_id_tbl_p->free_idx_bitmap[0]);
    return 0;
}

void score_id_tbl_flush_by_dcp(uint8_t dcp_id)
{
    int idx =0;
    score_stream_id_obj_t *stream_obj;
    score_mon_stop_data_t stream_mon_stop_cfg={};
    bool dcp_id_en = false;

    for (idx=0;idx < STREAM_ID_MAX_ENTRIES; idx++)
    {
        STREAM_ID_TBL_LOCK();
        stream_obj = &stream_id_tbl_p->entry[idx];
        if(stream_obj->in_use && IS_BIT_ON(stream_obj->dcp_bmap, dcp_id)) {
            dcp_id_en = true;
            stream_mon_stop_cfg.dcp_id = dcp_id;
            stream_mon_stop_cfg.score_stream_id = stream_obj->id.key;
            __logDebug("streamid:[%u] incarn:[%d] dcp_bmap=%x ct=%px \n",stream_obj->id.idx,
                    stream_obj->id.incarn, stream_obj->dcp_bmap, stream_obj->ct);
        } else {
            dcp_id_en = false;
        }
        STREAM_ID_TBL_UNLOCK();

        if(dcp_id_en) 
            score_monitor_stop_by_streamid(&stream_mon_stop_cfg);
    }
}
/*
 * Function to dump mapping table. 
 */
void score_id_tbl_dump(bool dump_all)
{
    int idx =0;
    score_stream_id_obj_t *stream_obj;

    STREAM_ID_TBL_LOCK();
    for (idx=0;idx < STREAM_ID_MAX_ENTRIES; idx++)
    {
        stream_obj = &stream_id_tbl_p->entry[idx];
        if(dump_all || stream_obj->in_use)
        {
            __logNotice("streamid:[%u] incarn:[%d] dcp_bmap=%x ct=%px \n",stream_obj->id.idx,
                    stream_obj->id.incarn, stream_obj->dcp_bmap, stream_obj->ct);
        }
    }
    STREAM_ID_TBL_UNLOCK();
    return;
}
EXPORT_SYMBOL(score_id_tbl_dump);

static inline void score_ct_dump_tuple(const struct nf_conntrack_tuple *t)
{
    switch (t->src.l3num) {
        case AF_INET:
            __logDebug("tuple ipv4: %u %pI4:%hu -> %pI4:%hu\n",
                    t->dst.protonum,
                    &t->src.u3.ip, ntohs(t->src.u.all),
                    &t->dst.u3.ip, ntohs(t->dst.u.all));
            break;
        case AF_INET6:
            __logDebug("ct_tuple ipv6: %u %pI6 %hu -> %pI6 %hu\n",
                    t->dst.protonum,
                    t->src.u3.all, ntohs(t->src.u.all),
                    t->dst.u3.all, ntohs(t->dst.u.all));
            break;
    }
}

static inline int score_stream_tuple_to_ct_tuple(score_stream_tuple_t *st_tuple,
        struct nf_conntrack_tuple *ct_tuple)
{
    if (st_tuple->family == SCORE_FAMILY_IPV4) {
        ct_tuple->src.l3num = AF_INET;
        ct_tuple->src.u3.ip = st_tuple->source_ip_addr.ipv4.u32;
        ct_tuple->dst.u3.ip = st_tuple->dest_ip_addr.ipv4.u32;

    } else if (st_tuple->family == SCORE_FAMILY_IPV6) {
        ct_tuple->src.l3num = AF_INET6;
        memcpy(ct_tuple->src.u3.ip6, &st_tuple->source_ip_addr.ipv6,
                sizeof(ct_tuple->src.u3.ip6));
        memcpy(ct_tuple->dst.u3.ip6, &st_tuple->dest_ip_addr.ipv6,
                sizeof(ct_tuple->dst.u3.ip6));
    } else {
        __logNotice("%s: unsupported l3 protoccol %x\n", __func__, st_tuple->family);
        return -1;
    }

    ct_tuple->src.u.all = st_tuple->source_port;
    ct_tuple->dst.u.all = st_tuple->dest_port;
    ct_tuple->dst.protonum = st_tuple->proto;

    score_ct_dump_tuple(ct_tuple);
    //ct_tuple->dst.dir = st_tuple->dir; /*make sure this is valid */
    return 0;
}

static inline int score_ct_tuple_to_stream_tuple(struct nf_conntrack_tuple *ct_tuple,
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

/* this function increments refcount on ct,
 * caller has to explicitly call nf_ct_put(ct) after done with ct usage
 */
struct nf_conn *stream_tuple_to_ct_get(score_stream_tuple_t *st_tuple, uint16_t zone_id)
{
#if 1
    struct nf_conntrack_tuple_hash *h;
    struct nf_conntrack_tuple ct_tuple;
    struct nf_conntrack_zone zone;
    struct net *net = current->nsproxy->net_ns;
    //struct net *net = &init_net;
    struct nf_conn *ct;

    memset(&ct_tuple, 0, sizeof(ct_tuple));
    /* covert to ct tuple format */
    if (score_stream_tuple_to_ct_tuple(st_tuple, &ct_tuple))
        return NULL;

    nf_ct_zone_init(&zone, NF_CT_DEFAULT_ZONE_ID,
            NF_CT_DEFAULT_ZONE_DIR, 0);
#if 1
    ct_tuple.dst.dir = IP_CT_DIR_ORIGINAL;
#else
    /* TODO, use zone_id,  for now just using default zone*/
#ifdef CONFIG_NF_CONNTRACK_ZONES
    //zone.id = zone_id;
    //zone->dir = NF_CT_ZONE_DIR_ORIG;
#endif
#endif

    h = nf_conntrack_find_get(net, &zone, &ct_tuple);
    if (!h)
    {
        __logDebug("find conntrack failed\n");
        return NULL;
    }

    ct = nf_ct_tuplehash_to_ctrack(h);
    return ct;
#else
    return NULL;
#endif
}


void destroy_dcp_bmap(score_stream_id_obj_t *stream_obj)
{
    uint16_t valid_bmap = stream_obj->dcp_bmap;
    void *dcp_priv;
    int dcp_id;
    score_mon_stop_data_t stream_mon_stop_cfg = {};

    for (dcp_id=0; dcp_id<SCORE_MAX_DCP_NUM; dcp_id++) {

        if (IS_BIT_ON(valid_bmap, dcp_id))
        {
            dcp_priv = stream_obj->dcp_priv[dcp_id];
            stream_mon_stop_cfg.score_stream_id = stream_obj->id.key;
            stream_mon_stop_cfg.dcp_id = dcp_id;
            stream_mon_stop_cfg.notification_req = 1;
            score_dcp_notify(SCORE_DCP_CMD_STREAM_DELETE, &stream_mon_stop_cfg, &dcp_priv);
            stream_obj->dcp_priv[dcp_id] = NULL;
        }
    }
    stream_obj->dcp_bmap = 0;
}


void update_dcp_bmap(score_stream_id_obj_t *stream_obj, uint16_t req_dcp_bmap)
{
#if 0
    uint16_t changed_bmap = stream_obj->dcp_bmap ^ req_dcp_bmap;
#endif
}

static inline void streamid_destroy_by_ct(struct nf_conn *ct)
{
    score_stream_id_obj_t *stream_obj;
    uint16_t streamkey=ct->bcm_ext.score_stream_id;

    stream_obj = streamid_to_streamobj(streamkey);
    if (stream_obj) {
        if (score_streamid_destroy_on_ct_destroy) { 
            destroy_dcp_bmap(stream_obj);
            free_streamid(streamkey);
        } else {
            stream_obj->ct=NULL;
        }
    }
    ct->bcm_ext.score_stream_id = SCORE_STREAM_ID_INVALID;
    ct->bcm_ext.score_dcp_bitmask = 0;
}

static inline bool bcm_tcp_get_opt_ts(const struct tcphdr *th)
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
                    return true;
                }
                ptr += opsize-2;
                length -= opsize;
        }
    }
    return false;
}

static inline bool bcm_tcp_get_opt_ts_aligned(const struct tcphdr *th)
{
    const __be32 *ptr = (const __be32 *)(th + 1);

    if (*ptr == htonl((TCPOPT_NOP << 24) | (TCPOPT_NOP << 16)
                | (TCPOPT_TIMESTAMP << 8) | TCPOLEN_TIMESTAMP)) {
        return true;
    }
    return false;
}

static bool bcm_tcp_parse_option_ts(const struct tcphdr *th)
{
    if (th->doff == (sizeof(*th) / 4)) {
        return false;
    } else if (th->doff == ((sizeof(*th) + TCPOLEN_TSTAMP_ALIGNED) / 4)) {
        if (bcm_tcp_get_opt_ts_aligned(th))
            return true;
    }
    return bcm_tcp_get_opt_ts(th);
}

static inline bool skb_has_tcp_timestamp(struct nf_conn *ct, struct sk_buff *skb)
{
    const struct nf_conntrack_tuple *t = nf_ct_tuple(ct, IP_CT_DIR_ORIGINAL);
    struct tcphdr *th = (struct tcphdr *)skb_transport_header(skb);
    if(t->dst.protonum == IPPROTO_TCP)
        return bcm_tcp_parse_option_ts(th);
    else 
        return false;
}

static inline bool __is_multicast_addr(const struct nf_conntrack_tuple *t)
{
    bool is_multicast = false;

    switch (t->src.l3num) {
        case AF_INET:
            is_multicast = ipv4_is_multicast(t->dst.u3.ip);
            break;
        case AF_INET6:
            is_multicast = ipv6_addr_is_multicast((const struct in6_addr *)t->dst.u3.all);
            break;
        default:
            __logNotice("%s: unsupported l3 protoccol %x\n", __func__, ntohs(t->src.l3num));
            break;
    }
    return is_multicast;
}

static inline int is_ct_score_not_supported(struct nf_conn *ct)
{
    const struct nf_conntrack_tuple *t = nf_ct_tuple(ct, IP_CT_DIR_ORIGINAL);
    bool is_multicast = __is_multicast_addr(t);
    int ret = SCORE_ERR_OK;

    /*support only TCP & UDP, GRE */
    if (is_multicast) {
        __logDebug(" CT not supported for multicast\n");
        ret = SCORE_ERR_STREAM_MCAST;
    } else if ((t->dst.protonum != IPPROTO_TCP) && (t->dst.protonum != IPPROTO_UDP)
            && (t->dst.protonum != IPPROTO_GRE)) {
        __logDebug(" CT not supported l4 protocol=%u \n", t->dst.protonum);
        ret = SCORE_ERR_STREAM_UNSUPPORTED_L4PROTO;
    }
    return ret;
}

static inline int streamid_create_by_ct(struct nf_conn *ct,
        uint8_t dcp_id, score_mon_start_data_t *stream_mon_start_cfg, Blog_t *blog)
{
    stream_id_key_t stream_id;
    score_stream_id_obj_t *stream_obj=NULL;
    void *dcp_priv;
    int ret = SCORE_ERR_OK;

    stream_id.key = alloc_streamid(&stream_obj);

    if(stream_id.key == SCORE_STREAM_ID_INVALID) {
        return  SCORE_ERR_STREAM_ID_ALLOC_FAIL;
    }

    stream_mon_start_cfg->ct = ct;

    dcp_priv = NULL;
    stream_mon_start_cfg->score_stream_id = stream_obj->id.key;
    stream_mon_start_cfg->dcp_id = dcp_id;
    ret = score_dcp_notify(SCORE_DCP_CMD_STREAM_ADD,
            stream_mon_start_cfg, &dcp_priv);
    if (ret == SCORE_ERR_OK) {
        SET_BIT(stream_obj->dcp_bmap, dcp_id);
        stream_obj->dcp_priv[dcp_id] = dcp_priv;

        ct->bcm_ext.score_stream_id = stream_obj->id.key;
        ct->bcm_ext.score_dcp_bitmask = stream_obj->dcp_bmap;
        stream_obj->ct = ct;
    } else {
        /*free stream id */
        stream_mon_start_cfg->score_stream_id = SCORE_STREAM_ID_INVALID;
        __logNotice(" stream not valid,  key: 0x%x dcp: %u ret=%d \n",
                stream_id.key, dcp_id, ret);
        free_streamid(stream_obj->id.key);
    }
    return ret;
}


static inline int streamid_update_by_ct(struct nf_conn *ct,
        uint8_t dcp_id, score_mon_start_data_t *stream_mon_start_cfg, bool is_dcp_add)
{
    stream_id_key_t stream_id;
    score_stream_id_obj_t *stream_obj=NULL;
    bool cur_dcp_id_bit_val = false;
    void *dcp_priv;
    int ret;

    stream_id.key = ct->bcm_ext.score_stream_id;
    stream_obj = streamid_to_streamobj(stream_id.key);

    if(!stream_obj){
        return SCORE_STREAM_ID_INVALID;
    }

    cur_dcp_id_bit_val = IS_BIT_ON(stream_obj->dcp_bmap, dcp_id);

    if(is_dcp_add ) {
        /* check if dcp_id is already set */
        if(cur_dcp_id_bit_val) {
            __logNotice(" dcp_id =%u already set for streamid=%u\n", dcp_id, stream_id.key);
            ret = SCORE_STREAM_ID_INVALID;
            goto error;
        } 

        /* add dcp for this stream */ 
        dcp_priv = NULL;
        stream_mon_start_cfg->score_stream_id = stream_obj->id.key;
        stream_mon_start_cfg->dcp_id = dcp_id;
        ret = score_dcp_notify(SCORE_DCP_CMD_STREAM_ADD, stream_mon_start_cfg, &dcp_priv);
        if (ret != SCORE_ERR_OK) {
            stream_mon_start_cfg->score_stream_id = SCORE_STREAM_ID_INVALID;
            goto error;
        }
        SET_BIT(stream_obj->dcp_bmap, dcp_id);
        stream_obj->dcp_priv[dcp_id] = dcp_priv;

    } else {
        score_mon_stop_data_t stream_mon_stop_cfg;
        /* check if dcp_id is not set */
        if(!cur_dcp_id_bit_val) {
            __logNotice(" dcp_id =%u already cleared for streamid=%u\n", dcp_id, stream_id.key);
            ret = SCORE_STREAM_ID_INVALID;
            goto error;
        } 
        /* delete dcp for this stream */ 
        dcp_priv = stream_obj->dcp_priv[dcp_id];
        stream_mon_stop_cfg.score_stream_id = stream_obj->id.key;
        stream_mon_stop_cfg.dcp_id = dcp_id;
        ret = score_dcp_notify(SCORE_DCP_CMD_STREAM_DELETE, &stream_mon_stop_cfg, &dcp_priv);

        CLR_BIT(stream_obj->dcp_bmap, dcp_id);
        stream_obj->dcp_priv[dcp_id] = NULL;
    }

    ct->bcm_ext.score_dcp_bitmask = stream_obj->dcp_bmap;
error:
    return ret;
}

#if IS_ENABLED(CONFIG_BCM_NFT_SCORE)
static inline int score_get_dcp_cfg_from_ct_and_start_monitor(struct nf_conn *ct, score_mon_start_data_t *stream_mon_start_cfg)
{
    ct_score_info_t *info = bcm_nfconn_ext_field_get(ct, score);
    unsigned long bmap = info->dcp_bmap;
    int dcp;
    
    for_each_set_bit(dcp, &bmap, MAX_SCORE_DCP_INFO_ENTRIES)
    {
        ct_score_entry_t    *entry = &(info->entries[dcp]);

        /* check if dcp is valid */
        if ((dcp >= SCORE_MAX_DCP_NUM) ||
                (dcp_db[dcp].info.dcp_id == SCORE_DCP_ID_INVALID)) {
            __logDebug("Invalid dcp_id=%u \n", dcp);
            continue;
        }

        stream_mon_start_cfg->dcp_id = dcp;
        if (dcp_db[dcp].info.dcp_type == DCP_TYPE_BNL)
        {
            stream_mon_start_cfg->stream_data.bnl_cfg.use_bincfg_ids = 1;
            stream_mon_start_cfg->stream_data.bnl_cfg.rtt_orig_bincfg_id = entry->orig_binidx;
            stream_mon_start_cfg->stream_data.bnl_cfg.rtt_reply_bincfg_id = entry->reply_binidx;
            stream_mon_start_cfg->stream_data.bnl_cfg.app_group_id = entry->app_group_id;
        }
        else
            stream_mon_start_cfg->stream_data.qstat_profile = 0;

        score_monitor_start_by_streamtuple(stream_mon_start_cfg);
        __logDebug("CT new event dcp_id=0x%x", dcp);
    }
    return 0;
}
#endif

static int
bcm_score_ct_event(enum ip_conntrack_events events, struct nf_conn *ct, struct sk_buff *skb)
{
    /* on new connection & marked for scoretrace try to allocate a traced id
     * store trace id in ct and also upate in blog(if present)*/

    if (events & (1 << IPCT_DESTROY)) {
        uint16_t stream_id = 0;
        /*TODO stop tracking from time wait state instead of waiting for destroy */
        STREAM_ID_TBL_LOCK();
        if (ct->bcm_ext.score_stream_id) {
            stream_id = ct->bcm_ext.score_stream_id;
            streamid_destroy_by_ct(ct);
        }
        STREAM_ID_TBL_UNLOCK();
        if(stream_id)
            __logDebug("CT destroy event streamid=%u \n", stream_id);
    }
#if IS_ENABLED(CONFIG_BCM_NFT_SCORE)
    else if (events & ((1 << IPCT_NEW) | (1 << IPCT_RELATED))) {
        score_mon_start_data_t stream_mon_start_cfg;

        if(skb_has_tcp_timestamp(ct, skb))
            ct->bcm_ext.orig_tcp_ts_en = 1;

        if (!bcm_nfconn_ext_field_get(ct, score)) {

            /*TODO how to avoid multiple CT's for same pkt allocating
             * multiple score_stream_id's when blog is not present
             */
            return 0;
        }

        /*check if blog is present */
        if (skb->blog_p && skb->blog_p->score_info.score_key.idx) {
            /*already monitored by some other ct */
            return 0;
        }

        memset(&stream_mon_start_cfg, 0, sizeof(stream_mon_start_cfg));
        stream_mon_start_cfg.notification_req = 1;

        score_ct_tuple_to_stream_tuple(nf_ct_tuple(ct, IP_CT_DIR_ORIGINAL),
                &stream_mon_start_cfg.stream_tuple);
        score_get_dcp_cfg_from_ct_and_start_monitor(ct, &stream_mon_start_cfg);
    }
#endif
    else if (events & (1 << IPCT_REPLY)) {
        if(skb_has_tcp_timestamp(ct, skb))
            ct->bcm_ext.reply_tcp_ts_en = 1;
    }
    return 0;
}

int score_monitor_start_by_streamtuple(score_mon_start_data_t *stream_mon_start_cfg)
{
    struct nf_conn *ct;
    uint8_t dcp_id = stream_mon_start_cfg->dcp_id;
    int ret = SCORE_ERR_OK;

    stream_mon_start_cfg->score_stream_id = SCORE_STREAM_ID_INVALID;

    if ((dcp_id >= SCORE_MAX_DCP_NUM) || (dcp_db[dcp_id].info.dcp_id == SCORE_DCP_ID_INVALID)) {
        __logDebug("Invalid dcp_id=%u \n", dcp_id);
        return SCORE_ERR_DCP_ID_INVALID;
    }

    ct = stream_tuple_to_ct_get(&stream_mon_start_cfg->stream_tuple, 0);
    if (ct == NULL) {
        /*TODO do we need to define error codes for userapp*/
        __logNotice("ct with requested tuple not found\n");
        return SCORE_ERR_STREAM_CT_NOTFOUND;
    }

    ret = is_ct_score_not_supported(ct);
    if (ret != SCORE_ERR_OK) {
        goto done;
    }

    STREAM_ID_TBL_LOCK();
    stream_mon_start_cfg->ct = ct;
    if (ct->bcm_ext.score_stream_id) {
        /* stream_id exists, update dcp bmap */
        ret = streamid_update_by_ct(ct, stream_mon_start_cfg->dcp_id, stream_mon_start_cfg, true);
    } else {
        /* allocate stream_id */
        ret = streamid_create_by_ct(ct, stream_mon_start_cfg->dcp_id, stream_mon_start_cfg, NULL);
    }
    STREAM_ID_TBL_UNLOCK();

    if(stream_mon_start_cfg->score_stream_id != SCORE_STREAM_ID_INVALID) {
        /* update existing flows */
        blog_lock();
        blog_notify(UPDATE_SCOREINFO, (void*)ct, ct->bcm_ext.score_stream_id,
                ct->bcm_ext.score_dcp_bitmask);
        blog_unlock();
    }
done:
    nf_ct_put(ct);
    return ret;
}

int score_monitor_stop_by_streamid(score_mon_stop_data_t *stream_mon_stop_cfg)
{
    struct nf_conn *ct;
    score_stream_id_obj_t *stream_obj;
    void *dcp_priv;
    uint8_t dcp_id = stream_mon_stop_cfg->dcp_id;
    bool update_flows=false;
    uint16_t dcp_bmap=0;
    int ret = SCORE_ERR_OK;
    stream_id_key_t stream_id;
    stream_id.key = stream_mon_stop_cfg->score_stream_id;

    if ((dcp_id >= SCORE_MAX_DCP_NUM) || (dcp_db[dcp_id].info.dcp_id == SCORE_DCP_ID_INVALID)) {
        __logInfo("Invalid dcp_id=%u streamid=%u/[%u:%u] \n", dcp_id, stream_id.key, stream_id.incarn, stream_id.idx);
        return SCORE_ERR_DCP_ID_INVALID;
    }

    STREAM_ID_TBL_LOCK();
    stream_obj = streamid_to_streamobj(stream_mon_stop_cfg->score_stream_id);

    if(!stream_obj) {
        __logNotice("Invalid streamid=%u/[%u:%u] not present, already freed dcp_id=%u\n", stream_id.key, stream_id.incarn, stream_id.idx, dcp_id);
        ret = SCORE_ERR_STREAM_ID_INVALID;
        goto unlock_done;
    }

    if (!IS_BIT_ON(stream_obj->dcp_bmap, dcp_id)) {
        __logInfo("Invalid streamid=%u/[%u:%u] dcp_id=%u not set \n", stream_id.key, stream_id.incarn, stream_id.idx, dcp_id);
        ret = SCORE_ERR_STREAM_ID_INVALID;
        goto unlock_done;
    }

    ct=stream_obj->ct;
    if (ct) {
        /* refcnt is needed to avoid race between ct_destroy & user app 
         * with out refcnt ct become invalid after STREAM_ID_TBL_UNLOCK */
        if (likely(refcount_inc_not_zero(&ct->ct_general.use)))
            update_flows = true;

        streamid_update_by_ct(ct, dcp_id, NULL, false);
        dcp_bmap = stream_obj->dcp_bmap;
    } else {
        /* delayed delete mode, after CT destroy, no flows */
        dcp_priv = stream_obj->dcp_priv[dcp_id];
        score_dcp_notify(SCORE_DCP_CMD_STREAM_DELETE, &stream_mon_stop_cfg, &dcp_priv);
        CLR_BIT(stream_obj->dcp_bmap, dcp_id);
        stream_obj->dcp_priv[dcp_id] = NULL;

        dcp_bmap = stream_obj->dcp_bmap;
        /* no more active dcp's  free streadid */
        if (stream_obj->dcp_bmap == 0)
            free_streamid(stream_obj->id.key);
    }
unlock_done:
    STREAM_ID_TBL_UNLOCK();

    if (update_flows) {
        /* TODO change the order of blog_notfiy & dcp_notify for perflow sdr's */
        /* update existing flows of this ct */
        blog_lock();
        blog_notify(UPDATE_SCOREINFO, (void*)ct, ct->bcm_ext.score_stream_id,
                ct->bcm_ext.score_dcp_bitmask);
        blog_unlock();

        /* if no more active dcp's release streamid */
        STREAM_ID_TBL_LOCK();
        stream_obj = streamid_to_streamobj(stream_mon_stop_cfg->score_stream_id);
        /* check both local dcp_bmap & stream_obj->dcp_bmap to maintain delete order  
         *  as  lock was released after updating local dcp_bmap 
         */
        if (stream_obj && (dcp_bmap == 0) && (stream_obj->dcp_bmap ==0)) {
            free_streamid(stream_obj->id.key);
            ct->bcm_ext.score_stream_id = SCORE_STREAM_ID_INVALID;
            ct->bcm_ext.score_dcp_bitmask = 0;
        }
        STREAM_ID_TBL_UNLOCK();
        nf_ct_put(ct);
    }
    return ret;
}

uint16_t score_monitor_get_streamid(score_stream_tuple_t *stuple, uint8_t dcp_id)
{
    struct nf_conn *ct;
    uint16_t streamkey=SCORE_STREAM_ID_INVALID;
    score_stream_id_obj_t *stream_obj=NULL;

    if ((dcp_id >= SCORE_MAX_DCP_NUM) || (dcp_db[dcp_id].info.dcp_id == SCORE_DCP_ID_INVALID)) {
        __logDebug("Invalid dcp_id=%u \n", dcp_id);
        return streamkey;
    }

    ct = stream_tuple_to_ct_get(stuple, 0);
    if (ct == NULL)
        return streamkey;

    STREAM_ID_TBL_LOCK();
    if (ct->bcm_ext.score_stream_id)
        stream_obj = streamid_to_streamobj(ct->bcm_ext.score_stream_id);

    if (stream_obj) {/* return stream id onli if dcp_id is valid */
        if (IS_BIT_ON(stream_obj->dcp_bmap, dcp_id))
            streamkey=ct->bcm_ext.score_stream_id;
    }
    STREAM_ID_TBL_UNLOCK();
    nf_ct_put(ct);
    return streamkey;
}

#if 0

/* should avoid using this better to use stop by_streamid */
int score_monitor_stop_by_streamtuple(score_stream_tuple_t *stuple, uint8_t dcp_id)
{
    struct nf_conn *ct;
    ct = stream_tuple_to_ct_get(stuple);
    if (ct == NULL)
    {
        /*TODO define error codes */
        return -1;
    }
    nf_ct_put(ct);
    return 0;
}

struct nf_conn * streamid_to_ct(uint16_t streamid)
{
    /* better not to use this, as we need to maintain refcnt's ect..
     * if needed will add later
     */ 
    return NULL;
}
#endif

static inline bool is_bnl_dcp_present(uint16_t dcp_bmap)
{
    int i;

    for (i = 0; i < SCORE_MAX_DCP_NUM; i++)
    {
        if(IS_BIT_ON(dcp_bmap, i) && (dcp_db[i].info.dcp_type == DCP_TYPE_BNL)) 
            return true;
    }
    return false;
}

static inline bool is_score_sw_only(uint16_t dcp_bmap)
{
#if defined(CC_SW_ONLY_SCORE_SUPPORT)
	return true;
#else
	return false;
#endif
}

int score_blog_flow_notify(blog_score_flow_info_t *score_flow_info)
{
    struct nf_conn *ct;
    score_stream_id_obj_t *stream_obj=NULL;
    stream_id_key_t stream_id;
    int ret;

    __logDebug("cmd=%d streamid[%d:%d]\n", score_flow_info->cmd,
            score_flow_info->score_key.incarn, score_flow_info->score_key.idx);
    STREAM_ID_TBL_LOCK();
    if (score_flow_info->cmd == BLOG_SCORE_FLOW_ADD) {
        ct = score_flow_info->ct;

        if (ct && ct->bcm_ext.score_stream_id) {
            stream_obj = streamid_to_streamobj(ct->bcm_ext.score_stream_id);

            if (stream_obj) {
                /*call call back for perflow needed by qstat */ 
                ret = score_dcp_flow_notify(stream_obj->id.key, stream_obj->dcp_bmap,
                        0, score_flow_info);

                if(ret == 0) {
                    /*return stream id & dcp_map only on success */
                    score_flow_info->score_key.key = stream_obj->id.key;
                    score_flow_info->score_dcp_bitmask = stream_obj->dcp_bmap;

                    score_flow_info->is_score_bnl_dcp_en = is_bnl_dcp_present(stream_obj->dcp_bmap );
                    score_flow_info->is_score_sw_only_en = is_score_sw_only(stream_obj->dcp_bmap );
                }
            }
        }
        /*TODO ct does not have streamid, try to allocate one in defered scoreid alloc mode */
    } else if (score_flow_info->cmd == BLOG_SCORE_FLOW_DELETE) {

        /*clear direction */
        score_flow_info->score_key.dir = 0;
        stream_id.key = score_flow_info->score_key.key;
        stream_obj = streamid_to_streamobj(stream_id.key);

        if (stream_obj) {
            /*call callback for perflow needed by qstat */ 
            ret = score_dcp_flow_notify(stream_obj->id.key, 0, stream_obj->dcp_bmap,
                    score_flow_info);
        } else {
            __logDebug("stream obj not found for flow delete notifier\n");
        }

    } else if (score_flow_info->cmd == BLOG_SCORE_FLOW_UPDATE) {

        ct = score_flow_info->ct;
        if (ct && ct->bcm_ext.score_stream_id) {
            stream_obj = streamid_to_streamobj(ct->bcm_ext.score_stream_id);
        } else if (score_flow_info->score_key.key) {
                /* delete of all dcp's */
            score_flow_info->score_key.dir = 0;
            stream_id.key = score_flow_info->score_key.key;
            stream_obj = streamid_to_streamobj(stream_id.key);
        }

        if (stream_obj) {

            uint16_t add_dcp_bmap = ((stream_obj->dcp_bmap ^ score_flow_info->score_dcp_bitmask)
                    & stream_obj->dcp_bmap);
            uint16_t del_dcp_bmap = ((stream_obj->dcp_bmap ^ score_flow_info->score_dcp_bitmask)
                    & score_flow_info->score_dcp_bitmask);

            /* callback for per flow needed by qstat */ 
            ret = score_dcp_flow_notify(stream_obj->id.key, add_dcp_bmap, del_dcp_bmap, score_flow_info);

            if(ret == 0) {
                /*return streamid & dcp_map only on success */
                score_flow_info->score_dcp_bitmask = stream_obj->dcp_bmap;
                if(score_flow_info->score_dcp_bitmask) {
                    score_flow_info->score_key.key = stream_obj->id.key;
                    score_flow_info->is_score_bnl_dcp_en = is_bnl_dcp_present(stream_obj->dcp_bmap );
                    score_flow_info->is_score_sw_only_en = is_score_sw_only(stream_obj->dcp_bmap );
                } else {
                    score_flow_info->score_key.key = 0;
                    score_flow_info->is_score_bnl_dcp_en = 0;
                    score_flow_info->is_score_sw_only_en = 0;
                }
            }
        } else {
            /* no stream object clear stream_id & dcp_mask in blog */
            score_flow_info->score_dcp_bitmask = 0;
            score_flow_info->score_key.key = 0;
            score_flow_info->is_score_bnl_dcp_en = 0;
            score_flow_info->is_score_sw_only_en = 0;
        }
    } else if (score_flow_info->cmd == BLOG_SCORE_FLOW_HW_DEFER_ADD) {

        ct = score_flow_info->ct;

        if (ct && ct->bcm_ext.score_stream_id) {
            __logDebug("defered hw flow activate for score stream_id=%u\n",
                    ct->bcm_ext.score_stream_id);
            stream_obj = streamid_to_streamobj(ct->bcm_ext.score_stream_id);

            if (stream_obj) {
                /* perflow callabck needed by qstat */ 
                ret = score_dcp_flow_notify(stream_obj->id.key, stream_obj->dcp_bmap,
                        0, score_flow_info);

            }
        }
    } else {
        __logError("unknown command =%d\n", score_flow_info->cmd);
    }

    STREAM_ID_TBL_UNLOCK();
    return 0;
}

static inline void __score_blog_flow_accel_activate_event(struct nf_conn *ct,
        BlogFlowEventInfo_t *info)
{
    /* ensure ct is not being deleted */
    //if(likely(refcount_read(&ct->ct_general.use) >= 1))
    {
        if(info->flow_event_type == FLOW_EVENT_TYPE_HW) {
            /* TODO call score dcp notfiy with update info */
        }
    }
}

static inline void score_blog_flow_activate_event(BlogFlowEventInfo_t *info)
{
    int idx;
    struct nf_conn *ct;

    if ((info->flow_event_type == FLOW_EVENT_TYPE_FC) || 
            (info->flow_event_type == FLOW_EVENT_TYPE_HW)){

        for (idx=0; idx < info->ct_count; idx++)
        {
            ct = info->ct_p[idx];
            if ((ct == NULL) || (ct->bcm_ext.score_stream_id))
                continue;

            __score_blog_flow_accel_activate_event(ct, info);
        }
    }
}

static inline void __score_blog_flow_accel_deactivate_event(struct nf_conn *ct,
        BlogFlowEventInfo_t *info)
{
    /* ensure ct is not being deleted */
    //if(likely(refcount_read(&ct->ct_general.use) >= 1))
    {
        if(info->flow_event_type == FLOW_EVENT_TYPE_HW) {

            /* call score dcp notfiy */
        }
    }
}

static inline void score_blog_flow_deactivate_event( BlogFlowEventInfo_t *info)
{
    int idx;
    struct nf_conn *ct;

    if ((info->flow_event_type == FLOW_EVENT_TYPE_FC) || 
            (info->flow_event_type == FLOW_EVENT_TYPE_HW)){

        for (idx=0; idx < info->ct_count; idx++)
        {
            ct = info->ct_p[idx];
            if ((ct == NULL) || (ct->bcm_ext.score_stream_id))
                continue;

            __score_blog_flow_accel_deactivate_event(ct, info);
        }
    }
}

static int score_blog_flowevent_notify(struct notifier_block * nb,
        unsigned long  event, void *info)
{
    switch(event){

        case FLOW_EVENT_ACTIVATE:
            score_blog_flow_activate_event(info);
            break;

        case FLOW_EVENT_DEACTIVATE:
            score_blog_flow_deactivate_event(info);
            break;

        default:
            break;

    }
    return NOTIFY_OK;
}

static struct notifier_block score_blog_flowevent_notifier = {
    .notifier_call = score_blog_flowevent_notify,
};

int score_ct_init(void)
{
    stream_id_tbl_init();
    /* coverity[Out-of-bounds read:SUPPRESS] */
    rcu_assign_pointer(bcm_scoretrace_ct_event_fn, bcm_score_ct_event);
    /* coverity[Out-of-bounds read:SUPPRESS] */
    rcu_assign_pointer(blog_score_flow_notify_cb, score_blog_flow_notify);
    blog_flowevent_register_notifier(&score_blog_flowevent_notifier);
    synchronize_rcu();
    return 0;
}

void score_ct_exit(void)
{
    blog_flowevent_unregister_notifier(&score_blog_flowevent_notifier);
    /* coverity[Out-of-bounds read:SUPPRESS] */
    rcu_assign_pointer(bcm_scoretrace_ct_event_fn, NULL);
    /* coverity[Out-of-bounds read:SUPPRESS] */
    rcu_assign_pointer(blog_score_flow_notify_cb, NULL);
    synchronize_rcu();
}
