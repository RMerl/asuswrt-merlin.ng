#ifndef __BPM_CACHE_INLINE_H_INCLUDED__
#define __BPM_CACHE_INLINE_H_INCLUDED__

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define BPM_LOCK               (&bpm_pg->lock)
#define BPM_FLAGS_DEF()        unsigned long _flags
#define BPM_LOCK_IRQ()         spin_lock_irqsave(BPM_LOCK, _flags)
#define BPM_UNLOCK_IRQ()       spin_unlock_irqrestore(BPM_LOCK, _flags)
#define BPM_LOCK_BH()          spin_lock_bh(BPM_LOCK)
#define BPM_UNLOCK_BH()        spin_unlock_bh(BPM_LOCK)
#else
#define BPM_FLAGS_DEF()             /* */
#define BPM_LOCK_IRQ()         local_irq_disable()
#define BPM_UNLOCK_IRQ()       local_irq_enable()
#define BPM_LOCK()          local_bh_disable()
#define BPM_UNLOCK()        local_bh_enable()
#endif

#define BPM_STATS_ADD(u32, val)  (bpm_pg->bpm_stats.u32 += (val))
#define BPM_STATS_SUB(u32, val)  (bpm_pg->bpm_stats.u32 -= (val))
#define BPM_STATS_INCR(u32)      (bpm_pg->bpm_stats.u32++)
#define BPM_STATS_DECR(u32)      (bpm_pg->bpm_stats.u32--)
#define BPM_STATS_SET(u32,val)   (bpm_pg->bpm_stats.u32 = (val))
#define BPM_STATS_CLR(u32)       (bpm_pg->bpm_stats.u32 = 0)
#define BPM_STATS_GET(u32)       (bpm_pg->bpm_stats.u32)
#define BPM_WM_ADD(u32, val)  (bpm_pg->bpm_stats.wm.u32 += (val))
#define BPM_WM_SUB(u32, val)  (bpm_pg->bpm_stats.wm.u32 -= (val))
#define BPM_WM_INCR(u32)      (bpm_pg->bpm_stats.wm.u32++)
#define BPM_WM_DECR(u32)      (bpm_pg->bpm_stats.wm.u32--)
#define BPM_WM_SET(u32,val)   (bpm_pg->bpm_stats.wm.u32 = (val))
#define BPM_WM_CLR(u32)       (bpm_pg->bpm_stats.wm.u32 = 0)
#define BPM_WM_GET(u32)       (bpm_pg->bpm_stats.wm.u32)
#define CC_BPM_SKB_POOL_STATS

#if defined(CC_BPM_SKB_POOL_STATS)
#define BPM_SKB_POOL_STATS_ADD(u32, val)  (bpm_pg->bpm_stats.u32 += (val))
#define BPM_SKB_POOL_STATS_SUB(u32, val)  (bpm_pg->bpm_stats.u32 -= (val))
#define BPM_SKB_POOL_STATS_BPM_DEFS()     BPM_DEFS()
#else /* !defined(CC_BPM_SKB_POOL_STATS) */
#define BPM_SKB_POOL_STATS_ADD(u32, val)  do { } while (0)
#define BPM_SKB_POOL_STATS_SUB(u32, val)  do { } while (0)
#define BPM_SKB_POOL_STATS_BPM_DEFS()
#endif /* defined(CC_BPM_SKB_POOL_STATS) */

// #define CC_BPM_SKB_POOL_DBG_STATS

#if defined(CC_BPM_SKB_POOL_DBG_STATS)
#define BPM_SKB_POOL_DBG_STATS_ADD(u32, val)  (bpm_pg->bpm_stats.dbg_ ## u32 += (val))
#define BPM_SKB_POOL_DBG_STATS_SUB(u32, val)  (bpm_pg->bpm_stats.dbg_ ## u32 -= (val))
#define BPM_SKB_POOL_DBG_STATS_BPM_DEFS()     BPM_DEFS()
#else /* !defined(CC_BPM_SKB_POOL_DBG_STATS) */
#define BPM_SKB_POOL_DBG_STATS_ADD(u32, val)  do { } while (0)
#define BPM_SKB_POOL_DBG_STATS_SUB(u32, val)  do { } while (0)
#define BPM_SKB_POOL_DBG_STATS_BPM_DEFS()
#endif /* defined(CC_BPM_SKB_POOL_DBG_STATS) */

/* Helper macros for BPM Buf Pool management */
/* BPM_BUF_BASE_POOL_AVAIL_CNT provides number of buffer currently available in
 * base pool */
#define BPM_BUF_BASE_POOL_AVAIL_CNT(bpm)  ((bpm)->bpm_buf_pool.buf_top_idx - (bpm)->bpm_buf_pool.buf_cur_idx)
/* BPM_BUF_EXP_POOL_AVAIL_CNT provides number of buffer currently available in
 * expansion pool */
#define BPM_BUF_EXP_POOL_AVAIL_CNT(bpm)  ((bpm)->bpm_buf_pool.exp_buf_top_idx - (bpm)->bpm_buf_pool.exp_buf_cur_idx)
/* BPM_BUF_AVAIL_CNT provides number of buffers currently available in both pool */
#define BPM_BUF_AVAIL_CNT(bpm)        (BPM_BUF_BASE_POOL_AVAIL_CNT(bpm) + BPM_BUF_EXP_POOL_AVAIL_CNT(bpm))
/* BPM_BUF_EXPANSION_ROOM provides number of buffers that can be added to the
   pool by expansion */
#define BPM_BUF_EXPANSION_ROOM(bpm)       ((bpm)->max_dyn - ((bpm)->bpm_buf_pool.buf_top_idx + (bpm)->bpm_buf_pool.exp_buf_top_idx))
/* BPM_BUF_MAX_AVAIL_CNT provides number of buffers currently available in pool
   and buffers that can be added to the pool by expansion */
#define BPM_BUF_MAX_AVAIL_CNT(bpm)        (BPM_BUF_EXPANSION_ROOM(bpm) + BPM_BUF_AVAIL_CNT(bpm))

/* BPM_SKB_AVAIL_CNT provides number of skb currently available in skb pool */
#define BPM_SKB_AVAIL_CNT(bpm)        ((bpm)->bpm_skb_pool.skb_avail)
/* BPM_SKB_MAX_AVAIL_CNT provides number of skb currently available in pool
   and skbs that can be added to the pool by the expansion */
#define BPM_SKB_MAX_AVAIL_CNT(bpm)    ((bpm)->bpm_skb_pool.skb_avail + ((bpm)->bpm_skb_pool.skb_bound - (bpm)->bpm_skb_pool.skb_total))
/* BPM_SKB_INUSE_CNT provides number of skb's currently being used by modules */
#define BPM_SKB_INUSE_CNT(bpm)        ((bpm)->bpm_skb_pool.skb_total - (bpm)->bpm_skb_pool.skb_avail)

#define BPM_BUF_EXP_POOL_AVAIL_CNT(bpm)  ((bpm)->bpm_buf_pool.exp_buf_top_idx - (bpm)->bpm_buf_pool.exp_buf_cur_idx)
#define BPM_BUF_EXPANSION_ROOM(bpm)       ((bpm)->max_dyn - ((bpm)->bpm_buf_pool.buf_top_idx + (bpm)->bpm_buf_pool.exp_buf_top_idx))

/* Prefetch helper macros */
#define BPM_PREFETCH()                bcm_prefetch_ld(bpm_pg)
#define BPM_BUF_POOL_PREFETCH_LD(idx) bcm_prefetch_ld(&bpm_pg->bpm_buf_pool.buf_pool[idx])
#define BPM_BUF_POOL_PREFETCH_ST(idx) bcm_prefetch_st(&bpm_pg->bpm_buf_pool.buf_pool[idx])

/* Dynamic expansion shrink macros and helper function defs*/
#define IS_BPM_DYN_ALLOC_MODE() (bpm_pg->dyn_cfg.bpm_max_buf_count != bpm_pg->dyn_cfg.bpm_initial_buf_count)
#define IS_BPM_ALLOC_REACHED_MAX() ((bpm_pg->bpm_buf_pool.buf_top_idx + bpm_pg->bpm_buf_pool.exp_buf_top_idx) == \
        (bpm_pg->dyn_cfg.bpm_max_buf_count))
#define IS_BPM_SKB_DYN_ALLOC_MODE() (bpm_pg->dyn_cfg.bpm_max_skb_count != bpm_pg->dyn_cfg.bpm_initial_skb_count)
#define IS_BPM_SKB_ALLOC_REACHED_MAX() (bpm_pg->bpm_skb_pool.skb_total == bpm_pg->bpm_skb_pool.skb_bound)

#define IS_BUF_AVAIL_IN_BASE_POOL() (bpm_pg->bpm_buf_pool.buf_top_idx > bpm_pg->bpm_buf_pool.buf_cur_idx)
#define IS_BUF_AVAIL_IN_EXP_POOL() (bpm_pg->bpm_buf_pool.exp_buf_top_idx > bpm_pg->bpm_buf_pool.exp_buf_cur_idx)

#define BPM_BUF_POOL_CUR_IDX      (bpm_pg->bpm_buf_pool.buf_cur_idx)
#define BPM_BUF_POOL_TOP_IDX      (bpm_pg->bpm_buf_pool.buf_top_idx)
#define BPM_BUF_POOL_EXP_CUR_IDX  (bpm_pg->bpm_buf_pool.exp_buf_cur_idx)
#define BPM_BUF_POOL_EXP_TOP_IDX  (bpm_pg->bpm_buf_pool.exp_buf_top_idx)
#define BPM_SKB_POOL_AVAIL        (bpm_pg->bpm_skb_pool.skb_avail)
#define BPM_SKB_POOL_TOTAL        (bpm_pg->bpm_skb_pool.skb_total)
#define BPM_SKB_POOL_BOUND        (bpm_pg->bpm_skb_pool.skb_bound)
#define BPM_READ_POOL_STATS_CNT(x)  ((bpm_pg->bpm_stats.x) - (bpm_pg->last_bpm_stats.x))

static void bpm_buf_metadata_dump(const char *func, int line, void *buf_p);
static void bpm_dump_buf_status_counters(void);
static int bpm_buf_expand(uint32_t num_bufs);

static inline void _bpm_get_pool_stats(bpm_pool_stats_t *pusr_stats)
{
    BPM_DEFS();
    pusr_stats->buf_top_idx = BPM_STATS_GET(buf_top_idx);
    pusr_stats->buf_cur_idx = BPM_STATS_GET(buf_cur_idx);
    pusr_stats->exp_buf_top_idx = BPM_STATS_GET(exp_buf_top_idx);
    pusr_stats->exp_buf_cur_idx = BPM_STATS_GET(exp_buf_cur_idx);
    pusr_stats->buf_avail_cnt = BPM_STATS_GET(buf_avail_cnt);
    pusr_stats->buf_alloc_cnt = BPM_READ_POOL_STATS_CNT(buf_alloc_cnt);
    pusr_stats->buf_free_cnt  = BPM_READ_POOL_STATS_CNT(buf_free_cnt);
    pusr_stats->exp_buf_alloc_cnt = BPM_READ_POOL_STATS_CNT(exp_buf_alloc_cnt);
    pusr_stats->exp_buf_free_cnt = BPM_READ_POOL_STATS_CNT(exp_buf_free_cnt);
    pusr_stats->buf_fail_cnt = BPM_READ_POOL_STATS_CNT(buf_fail_cnt);
    pusr_stats->min_avail_cnt = BPM_STATS_GET(min_avail_cnt);
    pusr_stats->buf_expand_cnt = BPM_READ_POOL_STATS_CNT(buf_expand_cnt);
    pusr_stats->buf_shrink_cnt = BPM_READ_POOL_STATS_CNT(buf_shrink_cnt);
    pusr_stats->buf_exp_fail_cnt = BPM_READ_POOL_STATS_CNT(buf_exp_fail_cnt);
    pusr_stats->buf_inthr_alloc_cnt = BPM_READ_POOL_STATS_CNT(buf_inthr_alloc_cnt);
#if defined(CC_DBG_BPMBUF_PRISTINE)
    pusr_stats->buf_pristine_chk_fail_cnt = BPM_READ_POOL_STATS_CNT(buf_pristine_chk_fail_cnt);
#endif
    pusr_stats->skb_avail = BPM_STATS_GET(skb_avail);
    pusr_stats->skb_total = BPM_STATS_GET(skb_total);
    pusr_stats->skb_bound = BPM_STATS_GET(skb_bound);
    pusr_stats->skb_inthr_alloc_cnt = BPM_READ_POOL_STATS_CNT(skb_inthr_alloc_cnt);
    pusr_stats->skb_alloc_cnt = BPM_READ_POOL_STATS_CNT(skb_alloc_cnt);
    pusr_stats->skb_mult_alloc_cnt = BPM_READ_POOL_STATS_CNT(skb_mult_alloc_cnt);
    pusr_stats->skb_bpm_alloc_cnt = BPM_READ_POOL_STATS_CNT(skb_bpm_alloc_cnt);
    pusr_stats->skb_free_cnt = BPM_READ_POOL_STATS_CNT(skb_free_cnt);
    pusr_stats->skb_fast_free_cnt = BPM_READ_POOL_STATS_CNT(skb_fast_free_cnt);
    pusr_stats->skb_fail_cnt = BPM_READ_POOL_STATS_CNT(skb_fail_cnt);
    pusr_stats->skb_grow_cnt = BPM_READ_POOL_STATS_CNT(skb_grow_cnt);
    pusr_stats->skb_shrink_cnt = BPM_READ_POOL_STATS_CNT(skb_shrink_cnt);
#if defined(CC_DBG_BPMSKB_PRISTINE)
    pusr_stats->buf_alloc_cnt = BPM_READ_POOL_STATS_CNT(skb_pristine_chk_fail_cnt);
#endif
#if defined(CC_BPM_SKB_POOL_DBG_STATS)
    pusr_stats->dbg_skb_hdr_reset_cnt = BPM_READ_POOL_STATS_CNT(dbg_skb_hdr_reset_cnt);
    pusr_stats->dbg_shinfo_reset_cnt = BPM_READ_POOL_STATS_CNT(dbg_shinfo_reset_cnt);
    pusr_stats->dbg_full_cache_inv_cnt = BPM_READ_POOL_STATS_CNT(dbg_full_cache_inv_cnt);
    pusr_stats->dbg_part_cache_inv_cnt = BPM_READ_POOL_STATS_CNT(dbg_part_cache_inv_cnt);
#endif /* CC_BPM_SKB_POOL_DBG_STATS */
    pusr_stats->skb_hwbuf_attach_cnt = BPM_READ_POOL_STATS_CNT(skb_hwbuf_attach_cnt);
    pusr_stats->skb_hwbuf_free_cnt = BPM_READ_POOL_STATS_CNT(skb_hwbuf_free_cnt);
    /* WMs */
    pusr_stats->wm.max_buf_wm = BPM_WM_GET(max_buf_wm);
    pusr_stats->wm.max_inuse_buf_wm = BPM_WM_GET(max_inuse_buf_wm);
    pusr_stats->wm.min_skb_avail_cnt = BPM_WM_GET(min_skb_avail_cnt);
    pusr_stats->wm.max_skb_wm = BPM_WM_GET(max_skb_wm);
    pusr_stats->wm.max_inuse_skb_wm = BPM_WM_GET(max_inuse_skb_wm);
}

static inline void _bpm_update_wm(void)
{
    BPM_DEFS();
    BPM_WM_SET(max_buf_wm,(BPM_BUF_POOL_TOP_IDX + BPM_BUF_POOL_EXP_TOP_IDX));
    BPM_WM_SET(max_skb_wm, BPM_SKB_POOL_TOTAL);
    BPM_WM_SET(min_skb_avail_cnt,BPM_SKB_AVAIL_CNT(bpm_pg));
    BPM_WM_SET(max_inuse_buf_wm,0);
    BPM_WM_SET(max_inuse_skb_wm,0);
}

static inline void _bpm_update_stats_cor(int enable)
{
	BPM_DEFS();
	BPM_FLAGS_DEF();
	enable = enable ? 1 : 0;
    BPM_LOCK_IRQ();
	if (enable ^ bpm_pg->stats_cor)
	{
		/* change */
		bpm_pg->stats_cor = enable;
		if (!bpm_pg->stats_cor)
		{
			memset(&(bpm_pg->last_bpm_stats), 0, sizeof(bpm_pool_stats_t));
		}
	}
	BPM_UNLOCK_IRQ();
}

static inline int _bpm_get_stats_cor(void)
{
	BPM_DEFS();
	return bpm_pg->stats_cor;
}

static inline uint32_t _bpm_get_buf_dataref(void *buf_p)
{
    struct skb_shared_info *skb_shinfo = BPM_BUF_TO_PSHINFO(buf_p);
    return (atomic_read(&skb_shinfo->dataref));
}

static inline int _bpm_check_and_set_buf_state_free(void *buf_p, void *ptr, uint32_t buf_idx)
{
#if defined(CC_DBG_BPM_BUF_DOUBLE_FREE)
    if (!_is_buf_state_alloc(buf_p, ptr, buf_idx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_BPM, "Buffer State is incorrect "
                                      "Either Double Free or "
                                      "Buffer is corrupted or "
                                      "Non BPM Buffer");
        BCM_LOG_ERROR(BCM_LOG_ID_BPM, "buf:%px mark:%08x buf_state:%08x dataref:%08x",buf_p,
                _bpm_get_buf_mark(buf_p, ptr, buf_idx),
                _bpm_get_buf_state(buf_p, ptr, buf_idx),
                _bpm_get_buf_dataref(buf_p));
        dump_stack();
        return -1;
    }
    else 
        _bpm_set_buf_state_free(buf_p, ptr, buf_idx);
#endif /* CC_DBG_BPM_BUF_DOUBLE_FREE */
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * function   : _bpm_upd_dyn_buf_lo_thresh
 * description: updates the dynamic buffer low threshold
 *------------------------------------------------------------------------------
 */
static inline void _bpm_upd_dyn_buf_lo_thresh(void)
{
    BPM_DEFS();

    /* calc the low thresh for dynamic buffers in the global buffer pool */
    bpm_pg->dyn_buf_lo_thresh =
       (bpm_pg->max_dyn * BPM_PCT_DYN_BUF_LO_THRESH / 100);
}

static inline bool _bpm_start_soak_timer(void)
{
    bool ret;
    BPM_DEFS();
    BPM_FLAGS_DEF();
    BPM_LOCK_IRQ();
    if ((IS_BPM_DYN_ALLOC_MODE()) &&
        (!timer_pending(bpm_pg->soak_timer)) &&
        ((BPM_BUF_POOL_EXP_TOP_IDX > 0) ||
         (BPM_SKB_POOL_TOTAL > bpm_pg->dyn_cfg.bpm_initial_skb_count))) {
        bpm_pg->soak_timer->expires = jiffies +
                                     bpm_pg->dyn_cfg.bpm_soak_timer_duration;
        add_timer(bpm_pg->soak_timer);
        ret = true;
    }
    else
        ret = false;
    BPM_UNLOCK_IRQ();
    return ret;
}

static inline void _update_min_availability(void)
{
    BPM_DEFS();
    const uint32_t buf_avail_cnt = BPM_BUF_AVAIL_CNT(bpm_pg);

    if (unlikely(buf_avail_cnt < bpm_pg->dyn_cfg.bpm_buf_avail_lower_th))
        bpm_buf_expand_post_event();

    if (unlikely(BPM_STATS_GET(min_avail_cnt) > buf_avail_cnt))
        BPM_STATS_SET(min_avail_cnt,buf_avail_cnt);

    if (BPM_WM_GET(max_inuse_buf_wm) < (BPM_BUF_POOL_CUR_IDX + BPM_BUF_POOL_EXP_CUR_IDX))
        BPM_WM_SET(max_inuse_buf_wm,(BPM_BUF_POOL_CUR_IDX + BPM_BUF_POOL_EXP_CUR_IDX));
}

static inline void _update_skb_min_availability(void)
{
    BPM_DEFS();
    uint32_t total_inuse;
    uint32_t skb_avail_cnt = BPM_SKB_AVAIL_CNT(bpm_pg);
    if (unlikely(BPM_WM_GET(min_skb_avail_cnt) > skb_avail_cnt))
        BPM_WM_SET(min_skb_avail_cnt,skb_avail_cnt);
    if (unlikely(skb_avail_cnt <
                bpm_pg->dyn_cfg.bpm_skb_avail_lower_th)) {
        /* schedule monitor thread to add skb to skb freelist. */
        bpm_skb_expand_post_event();
    }
    total_inuse = BPM_SKB_INUSE_CNT(bpm_pg);
    if (total_inuse > BPM_WM_GET(max_inuse_skb_wm))
        BPM_WM_SET(max_inuse_skb_wm, total_inuse);
}

static inline void _add_buf_to_base_pool(void *buf_p, void *ptr, uint32_t buf_idx)
{
    BPM_DEFS();
    _bpm_set_buf_state_free(buf_p, ptr, buf_idx);
    bpm_pg->bpm_buf_pool.buf_pool[bpm_pg->bpm_buf_pool.buf_top_idx++] = buf_p;            /* POST ++ buf_top_idx */
    _bpm_buf_set_base_pool_mark(buf_p, ptr, buf_idx);
}

static inline void _add_buf_to_exp_pool(void *buf_p, void *ptr, uint32_t buf_idx)
{
    BPM_DEFS();
    _bpm_set_buf_state_free(buf_p, ptr, buf_idx);
    bpm_pg->bpm_buf_pool.exp_buf_pool[bpm_pg->bpm_buf_pool.exp_buf_top_idx++] = buf_p;    /* POST ++ exp_buf_top_idx */
    _bpm_buf_set_exp_pool_mark(buf_p, ptr, buf_idx);
}

static inline void *_release_buf_from_exp_pool(void)
{
    BPM_DEFS();
    return bpm_pg->bpm_buf_pool.exp_buf_pool[--bpm_pg->bpm_buf_pool.exp_buf_top_idx];     /* PRE -- exp_buf_top_idx */
}

static inline void _free_buf_to_base_pool(void *buf_p, void *ptr, uint32_t buf_idx)
{
    BPM_DEFS();
    if (!_bpm_check_and_set_buf_state_free(buf_p, ptr, buf_idx))
    {
        BPM_STATS_INCR(buf_free_cnt);
        bpm_pg->bpm_buf_pool.buf_pool[--bpm_pg->bpm_buf_pool.buf_cur_idx] = buf_p;            /* PRE -- buf_cur_idx */
    }
}

static inline void _free_buf_to_exp_pool(void *buf_p, void *ptr, uint32_t buf_idx)
{
    BPM_DEFS();
    if (!_bpm_check_and_set_buf_state_free(buf_p, ptr, buf_idx))
    {
        BPM_STATS_INCR(buf_free_cnt);
        BPM_STATS_INCR(exp_buf_free_cnt);
        bpm_pg->bpm_buf_pool.exp_buf_pool[--bpm_pg->bpm_buf_pool.exp_buf_cur_idx] = buf_p;    /* PRE -- exp_buf_cur_idx */
    }
}

static inline void *_alloc_buf_from_base_pool(void)
{
    void *buf_p;
    BPM_DEFS();
    uint32_t buf_idx;
    void *ptr;
    BPM_STATS_INCR(buf_alloc_cnt);
    buf_p = bpm_pg->bpm_buf_pool.buf_pool[bpm_pg->bpm_buf_pool.buf_cur_idx++];             /* POST ++ buf_cur_idx */
    ptr = _get_buf_md_and_idx(buf_p, &buf_idx);
    _bpm_set_buf_state_alloc(buf_p, ptr, buf_idx);
    return buf_p;
}

static inline void _alloc_mult_buf_from_base_pool(void **buf_arry, uint32_t num)
{
    BPM_DEFS();
    uint32_t buf_idx;
    void *ptr;
    uint32_t idx = 0;
    do {
        buf_arry[idx] = bpm_pg->bpm_buf_pool.buf_pool[bpm_pg->bpm_buf_pool.buf_cur_idx++];             /* POST ++ buf_cur_idx */
        ptr = _get_buf_md_and_idx(buf_arry[idx], &buf_idx);
        _bpm_set_buf_state_alloc(buf_arry[idx], ptr, buf_idx);
    }while(++idx < num);
    BPM_STATS_ADD(buf_alloc_cnt,num);
}

static inline void *_alloc_buf_from_exp_pool(void)
{
    void *buf_p;
    BPM_DEFS();
    uint32_t buf_idx;
    void *ptr;
    BPM_STATS_INCR(buf_alloc_cnt);
    BPM_STATS_INCR(exp_buf_alloc_cnt);
    buf_p = bpm_pg->bpm_buf_pool.exp_buf_pool[bpm_pg->bpm_buf_pool.exp_buf_cur_idx++];    /* POST ++ exp_buf_cur_idx */
    ptr = _get_buf_md_and_idx(buf_p, &buf_idx);
    _bpm_set_buf_state_alloc(buf_p, ptr, buf_idx);
    return buf_p;
}

static inline void _alloc_mult_buf_from_exp_pool(void **buf_arry, uint32_t num)
{
    BPM_DEFS();
    uint32_t buf_idx;
    void *ptr;
    uint32_t idx = 0;
    do {
        buf_arry[idx] = bpm_pg->bpm_buf_pool.exp_buf_pool[bpm_pg->bpm_buf_pool.exp_buf_cur_idx++];     /* POST ++ exp_buf_cur_idx */
        ptr = _get_buf_md_and_idx(buf_arry[idx], &buf_idx);
        _bpm_set_buf_state_alloc(buf_arry[idx], ptr, buf_idx);
    }while(++idx < num);
    BPM_STATS_ADD(buf_alloc_cnt,num);
    BPM_STATS_ADD(exp_buf_alloc_cnt,num);
}

static inline struct sk_buff *_bpm_get_skb_from_freelist(void)
{
    BPM_DEFS();
    struct sk_buff *skb = NULL;
    if (likely(BPM_SKB_POOL_AVAIL)) {
        bcm_prefetch(bpm_pg->bpm_skb_pool.skb_freelist);

        skb = bpm_pg->bpm_skb_pool.skb_freelist;
        BPM_STATS_INCR(skb_alloc_cnt);

        bpm_pg->bpm_skb_pool.skb_freelist = (struct sk_buff *)CLEAR_SKB_IN_BPM_SKBPOOL(skb->next);
        skb->next = (struct sk_buff *)NULL;
        --BPM_SKB_POOL_AVAIL;
        _update_skb_min_availability();
    } else {
        skb = NULL;
        BPM_STATS_INCR(skb_fail_cnt);
    }
    return skb;
}

static inline void _bpm_put_skb_to_freelist(struct sk_buff *skb)
{
    BPM_DEFS();
    /* SPC flag MUST be cleared by this point! */
    BCM_ASSERT(!skb->is_spc);
#if defined(CC_BPM_SKB_FREE_TO_TAIL)
    if (bpm_pg->bpm_skb_pool.skb_freelist == NULL)
    {
        BCM_ASSERT((BPM_SKB_POOL_AVAIL == 0));
        bpm_pg->bpm_skb_pool.skb_freelist      = skb;
        bpm_pg->bpm_skb_pool.skb_freelist_tail = skb;
        skb->next = NULL;
    }
    else
    {
        bpm_pg->bpm_skb_pool.skb_freelist_tail->next = (struct sk_buff *)MARK_SKB_IN_BPM_SKBPOOL(skb);
        bpm_pg->bpm_skb_pool.skb_freelist_tail = skb;
    }
#else
    skb->next = (struct sk_buff *)MARK_SKB_IN_BPM_SKBPOOL(bpm_pg->bpm_skb_pool.skb_freelist);
    bpm_pg->bpm_skb_pool.skb_freelist = skb;
#endif /* CC_BPM_SKB_FREE_TO_TAIL */
    ++BPM_SKB_POOL_AVAIL;
}

static inline void _bpm_put_skblist_to_freelist(struct sk_buff *head, struct sk_buff *tail, int count)
{
    BPM_DEFS();
#if defined(CC_BPM_SKB_FREE_TO_TAIL)
    if (bpm_pg->bpm_skb_pool.skb_freelist == NULL)
    {
        bpm_pg->bpm_skb_pool.skb_freelist       = head;
        bpm_pg->bpm_skb_pool.skb_freelist_tail = tail;
    }
    else
    {
        bpm_pg->bpm_skb_pool.skb_freelist_tail->next = (struct sk_buff *)MARK_SKB_IN_BPM_SKBPOOL(head);
        bpm_pg->bpm_skb_pool.skb_freelist_tail       = tail;
    }
#else
    tail->next = (struct sk_buff *)MARK_SKB_IN_BPM_SKBPOOL(bpm_pg->bpm_skb_pool.skb_freelist);
    bpm_pg->bpm_skb_pool.skb_freelist = head;
#endif /* CC_BPM_SKB_FREE_TO_TAIL */
    BPM_SKB_POOL_AVAIL += count;
    BPM_STATS_ADD(skb_free_cnt, count);
    BPM_STATS_ADD(skb_fast_free_cnt, count);
}

static inline void *_bpm_cache_alloc_buf(void)
{
    BPM_DEFS();
    BPM_FLAGS_DEF();
    void *buf_p;
    void *ptr;
    uint32_t buf_idx;
    BPM_PREFETCH();

    BPM_LOCK_IRQ();

    BPM_BUF_POOL_PREFETCH_LD(BPM_BUF_POOL_CUR_IDX);

    /* if buffers available in global buffer pool */
    if (likely(IS_BUF_AVAIL_IN_BASE_POOL())) {
        buf_p = _alloc_buf_from_base_pool();
        _update_min_availability();
    } else if (IS_BUF_AVAIL_IN_EXP_POOL()) {
        buf_p = _alloc_buf_from_exp_pool();
        _update_min_availability();
    } else if (!IS_BPM_ALLOC_REACHED_MAX()) {
        /* in-thread allocation, thus uisng GFP_ATOMIC flag */
        if ((buf_p = _bpm_construct_pdata(bpm_pg, GFP_ATOMIC)) == NULL) {
            BCM_LOG_ERROR(BCM_LOG_ID_BPM, "Kmem_cache_alloc failed to "
                                          "allocate a BPM buffer");
            BPM_STATS_INCR(buf_fail_cnt);
            goto alloc_exit;
        }
        BPM_STATS_INCR(buf_inthr_alloc_cnt);

        /* Save the data_p and not the in the buf_pool[] table */
        ptr = _get_buf_md_and_idx(buf_p, &buf_idx);
        _add_buf_to_exp_pool(buf_p, ptr, buf_idx);
        buf_p = _alloc_buf_from_exp_pool();
        _update_min_availability();
    } else {
        buf_p = NULL;
        BPM_STATS_INCR(buf_fail_cnt);
    }
alloc_exit:
    BPM_UNLOCK_IRQ();

    return buf_p;
}

static inline void _bpm_cache_free_buf(void *buf_p, void *ptr, uint32_t buf_idx)
{
    bool base_pool = false;
    BPM_DEFS();
    BPM_FLAGS_DEF();
    BPM_PREFETCH();

    if (unlikely(buf_p == NULL)) {
        BPM_STATS_INCR(buf_fail_cnt);
        BCM_LOG_ERROR(BCM_LOG_ID_BPM,"bpm free NULL buf_p\n");
        BCM_ASSERT(buf_p != NULL);
        return;
    }
#if defined(CC_DBG_BPMBUF_PRISTINE)
    /* Check if shinfo is clear */
    if (unlikely(!_is_buf_pristine(buf_p)))
    {
        BPM_STATS_INCR(buf_pristine_chk_fail_cnt);
    }
#endif

    BPM_LOCK_IRQ();

    base_pool = _is_buf_from_base_pool(buf_p, ptr, buf_idx);

    BPM_BUF_POOL_PREFETCH_ST(BPM_BUF_POOL_CUR_IDX - 1);

    if (base_pool)
    {
        if (BPM_BUF_POOL_CUR_IDX == 0U)
        {
            bpm_dump_buf_status_counters();
            bpm_buf_metadata_dump(__FUNCTION__, __LINE__, buf_p);
            BCM_ASSERT(BPM_BUF_POOL_CUR_IDX > 0U);
        }
        _free_buf_to_base_pool(buf_p, ptr, buf_idx);
    }
    else
    {
#if defined(CC_DBG_BPM_BUF_EXPANSION_POOL_UNDERRUN)
        if (unlikely(BPM_BUF_POOL_EXP_CUR_IDX == 0U))
        {
            bpm_dump_buf_status_counters();
            bpm_buf_metadata_dump(__FUNCTION__, __LINE__, buf_p);
            BCM_ASSERT(BPM_BUF_POOL_EXP_CUR_IDX > 0U);
        }
#endif   /* CC_DBG_BPM_BUF_EXPANSION_POOL_UNDERRUN */
        _free_buf_to_exp_pool(buf_p, ptr, buf_idx);
    }

    BPM_UNLOCK_IRQ();
}

static inline int _bpm_cache_alloc_mult_buf(uint32_t num, void **buf_p, uint32_t prio)
{
    int ret = BPM_SUCCESS;
    uint32_t _num = num;
    uint32_t expand_num, copy_cnt  = 0;
    uint32_t base_pool_avail_cnt, expn_pool_avail_cnt = 0;
    BPM_DEFS();
    BPM_FLAGS_DEF();
    BPM_PREFETCH();

    BCM_ASSERT(buf_p != NULL);
    BCM_ASSERT(num > 0); /* alloc_mult_buf_from_base_pool goes bad in case num == 0 */

    BPM_LOCK_IRQ();

    if (BPM_BUF_MAX_AVAIL_CNT(bpm_pg) < num)
    {
          BPM_UNLOCK_IRQ();
          return BPM_ERROR;
    }

    BPM_BUF_POOL_PREFETCH_LD(BPM_BUF_POOL_CUR_IDX);

    base_pool_avail_cnt = BPM_BUF_BASE_POOL_AVAIL_CNT(bpm_pg);
    expn_pool_avail_cnt = BPM_BUF_EXP_POOL_AVAIL_CNT(bpm_pg);

    if (base_pool_avail_cnt != 0)
    {
        copy_cnt = (base_pool_avail_cnt >= num)? num : base_pool_avail_cnt;
        _alloc_mult_buf_from_base_pool(&buf_p[0], copy_cnt);
        num -= copy_cnt;
        if (num == 0)
            goto exit_func;
    }

    if (unlikely((expn_pool_avail_cnt < num) && !IS_BPM_ALLOC_REACHED_MAX()))
    {
        expand_num = num - expn_pool_avail_cnt;
        /* Allocate more buffers. */
        if (expand_num > (BPM_BUF_EXPANSION_ROOM(bpm_pg)))
            expand_num = BPM_BUF_EXPANSION_ROOM(bpm_pg);

        BPM_STATS_ADD(buf_inthr_alloc_cnt,bpm_buf_expand(expand_num));
        expn_pool_avail_cnt = BPM_BUF_EXP_POOL_AVAIL_CNT(bpm_pg);
    }

    if (expn_pool_avail_cnt >= num)
    {
        _alloc_mult_buf_from_exp_pool(&buf_p[copy_cnt], num);
        goto exit_func;
    }

    /* We already allocated maximum number of BPM buffers */
    if (BPM_PRINT_FREQ(BPM_STATS_GET(buf_fail_cnt)))
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "Failed to allocate %d buffers; buf_fail_cnt = %u",
                       _num, BPM_STATS_GET(buf_fail_cnt));
    }
    BPM_STATS_INCR(buf_fail_cnt);
    ret = BPM_ERROR;
exit_func:
    _update_min_availability();
    BPM_UNLOCK_IRQ();
    return ret;
}

static inline void _bpm_cache_free_mult_buf(uint32_t num, void **buf_p)
{
    uint32_t buf_idx;
    bool base_pool = false;
    BPM_DEFS();
    BPM_FLAGS_DEF();
    void *ptr;
    uint32_t md_idx;
    BPM_PREFETCH();

    BCM_ASSERT(buf_p != NULL);

    BPM_LOCK_IRQ();

    BPM_BUF_POOL_PREFETCH_ST(BPM_BUF_POOL_CUR_IDX - 1);

    BPM_DBG_ASSERT(BPM_BUF_POOL_CUR_IDX >= num);


    for (buf_idx = 0; buf_idx < num; ++buf_idx) {
        ptr = _get_buf_md_and_idx(buf_p[buf_idx], &md_idx);
#if defined(CC_DBG_BPMBUF_PRISTINE)
        /* Check if shinfo is clear */
        if (unlikely(!_is_buf_pristine(buf_p[buf_idx])))
        {
            BPM_STATS_INCR(buf_pristine_chk_fail_cnt);
        }
#endif   /*CC_DBG_BPMBUF_PRISTINE*/

        base_pool = _is_buf_from_base_pool(buf_p[buf_idx], ptr, md_idx);

        if (base_pool)
        {
            BCM_ASSERT(BPM_BUF_POOL_CUR_IDX > 0U);
            _free_buf_to_base_pool(buf_p[buf_idx], ptr, md_idx);
        }
        else
        {
#if defined(CC_DBG_BPM_BUF_EXPANSION_POOL_UNDERRUN)
            if (unlikely(BPM_BUF_POOL_EXP_CUR_IDX == 0))
            {
                bpm_dump_buf_status_counters();
                BCM_ASSERT(BPM_BUF_POOL_EXP_CUR_IDX > 0U);
            }
#endif   /* CC_DBG_BPM_BUF_EXPANSION_POOL_UNDERRUN */
            _free_buf_to_exp_pool(buf_p[buf_idx], ptr, md_idx);
        }
        buf_p[buf_idx] = NULL;

    }
    BPM_UNLOCK_IRQ();
}

static inline void _bpm_cache_free_skb(void *skbp)
{
    struct sk_buff *skb = (struct sk_buff *)skbp;
    uint32_t hdr_rst_cnt = 0;
    BPM_DEFS();
    BPM_FLAGS_DEF();
    BPM_PREFETCH();

#if defined(CC_DBG_BPMSKB_PRISTINE)
    if (unlikely(!_is_skb_pristine(skb)))
    {
        //BCM_LOG_INFO(BCM_LOG_ID_BPM, "skb is not pristine");
        bpm_pg->skb_pristine_chk_fail_cnt++;
    }
#endif // CC_DBG_BPMSKB_PRISTINE

    /* if skb is not pristine so do a header reset or else just update the
     * recycle flags and hook */
    hdr_rst_cnt += _bpm_pristine_skb(skb, !IS_BPM_SKB_PRISTINE(skb));
    /* Add skb to BPM sk_buff Pool free list */

    BCM_ASSERT(!IS_SKB_IN_BPM_SKBPOOL(skb->next));
    BPM_LOCK_IRQ();
    _bpm_put_skb_to_freelist(skb);
    BPM_STATS_INCR(skb_free_cnt);
    BPM_SKB_POOL_DBG_STATS_ADD(skb_hdr_reset_cnt,hdr_rst_cnt);

    BPM_UNLOCK_IRQ();
} /* bpm_sw_free_skb() */

static inline void _bpm_cache_free_mult_skb(void *head, void *tail, uint32_t len, uint32_t rst_cnt)
{
#if defined(CC_DBG_BPMSKB_PRISTINE)
    struct sk_buff *skb;
#endif
    struct sk_buff *skb_head = (struct sk_buff *)head;
    struct sk_buff *skb_tail = (struct sk_buff *)tail;
    BPM_DEFS();
    BPM_FLAGS_DEF();
    BPM_PREFETCH();

#if defined(CC_DBG_BPMSKB_PRISTINE)
    for (skb = skb_head; skb != NULL; )
    {
        if (unlikely(!_is_skb_pristine(skb)))
        {
            //BCM_LOG_INFO(BCM_LOG_ID_BPM, "skb is not pristine");
            bpm_pg->skb_pristine_chk_fail_cnt++;
        }
        skb = skb->next;
    }
#endif   //CC_DBG_BPMSKB_PRISTINE

    BCM_LOG_DEBUG(BCM_LOG_ID_BPM,"len:%u\n",len);
    BPM_LOCK_IRQ();

    /* Add skb list to BPM sk_buff Pool free list */
    _bpm_put_skblist_to_freelist(skb_head, skb_tail, len);
    BPM_SKB_POOL_DBG_STATS_ADD(skb_hdr_reset_cnt,rst_cnt);

    BPM_UNLOCK_IRQ();

} /* bpm_sw_free_mult_skb */

static inline void *_bpm_cache_alloc_skb(void)
{
    struct sk_buff *skb;
    BPM_DEFS();
    BPM_FLAGS_DEF();
    BPM_PREFETCH();

    BPM_LOCK_IRQ();

    skb = _bpm_get_skb_from_freelist();

    BPM_UNLOCK_IRQ();

    return (void *)skb;
} /* bpm_sw_alloc_skb() */

static inline void *_bpm_cache_alloc_mult_skb(uint32_t request_num)
{
    struct sk_buff *skb, *skb_list;
    uint32_t alloc;
    BPM_DEFS();
    BPM_FLAGS_DEF();
    BPM_PREFETCH();

    BPM_DBG_ASSERT(request_num != 0U);

    BPM_LOCK_IRQ();


    if (unlikely(BPM_SKB_POOL_AVAIL < request_num)) {
        if ((BPM_SKB_POOL_TOTAL + request_num) <
             bpm_pg->dyn_cfg.bpm_max_skb_count) {
            /* Add to skb_freelist */
            BPM_UNLOCK_IRQ();
            /* Here the BPM SKB pool is expanded in thread, hence use
             * GFP_ATOMIC */
            BPM_STATS_ADD(skb_inthr_alloc_cnt, bpm_grow_skb_pool(request_num, GFP_ATOMIC));
            BPM_LOCK_IRQ();
        }
    }

    if (likely(BPM_SKB_POOL_AVAIL >= request_num)) {
        skb = _bpm_get_skb_from_freelist();

        skb_list = skb; /* list to be returned */

        for (alloc = 1; alloc < request_num; ++alloc) {
            skb->next = _bpm_get_skb_from_freelist();
            skb = (struct sk_buff *)skb->next;
        }
        skb->next = (struct sk_buff *)NULL;
        _update_skb_min_availability();
        BPM_STATS_ADD(skb_mult_alloc_cnt, request_num);
    } else {
        skb_list = NULL;
        BPM_STATS_INCR(skb_fail_cnt);
    }

    BPM_UNLOCK_IRQ();

    return (void *)skb_list;
} /* bpm_sw_alloc_mult_skb() */

static inline void *_bpm_cache_alloc_buf_skb_attach(uint32_t datalen)
{
    BPM_DEFS();
    BPM_FLAGS_DEF();
    void *buf;
    struct sk_buff *skb;
    bool base_pool = false;
    void *ptr;
    uint32_t md_idx;

    BPM_PREFETCH();

    /* Completely depleting SKB pool is a legit case
       TODO -- determine why this check was added as part of JIRA SWBCACPE-56920*/
    /* BCM_ASSERT(bpm_pg->skb_freelist != NULL); */

    BPM_LOCK_IRQ();

    /* Allocate buffer : reference bpm_sw_alloc_buf() */
    BPM_BUF_POOL_PREFETCH_LD(BPM_BUF_POOL_CUR_IDX);

    /* if buffers available in global buffer pool */
    if (likely(IS_BUF_AVAIL_IN_BASE_POOL())) {
        buf = _alloc_buf_from_base_pool();

        base_pool = true;
        _update_min_availability();
    } else if (likely(IS_BUF_AVAIL_IN_EXP_POOL())) {
        buf = _alloc_buf_from_exp_pool();

        _update_min_availability();
    } else {
        buf = NULL;
        BPM_STATS_INCR(buf_fail_cnt);

        BPM_UNLOCK_IRQ();

        return (void *)NULL;
    }

    /* Allocate SKB : reference bpm_alloc_skb() */
    skb = _bpm_get_skb_from_freelist();

    if (unlikely(skb == NULL)) {
        ptr = _get_buf_md_and_idx(buf, &md_idx);

        if (base_pool == true)
            _free_buf_to_base_pool(buf, ptr, md_idx);
        else
            _free_buf_to_exp_pool(buf, ptr, md_idx);

        BPM_UNLOCK_IRQ();

        return (void *)NULL;
    }

    BPM_UNLOCK_IRQ();

    BPM_STATS_INCR(skb_bpm_alloc_cnt);

    BPM_DBG_ASSERT(BPM_BUF_TO_PSHINFO(buf)->dirty_p == NULL);

#if defined(CC_DBG_BPMBUF_PRISTINE)
    /* Check if shinfo is clear */
    if (unlikely(!_is_buf_pristine(buf)))
    {
        struct skb_shared_info *skb_shinfo = BPM_BUF_TO_PSHINFO(buf);

        //BCM_LOG_INFO(BCM_LOG_ID_BPM,"skb shinfo is not clean");
        BPM_STATS_INCR(buf_pristine_chk_fail_cnt);
        skb_shinforeset(skb_shinfo); /* memset 0 and dataref = 1 */
        BPM_SKB_POOL_DBG_STATS_ADD(shinfo_reset_cnt, 1);
    }
#endif   //CC_DBG_BPMBUF_PRISTINE

#if defined(CC_DBG_BPMSKB_PRISTINE)
    if (unlikely(!_is_skb_pristine(skb)))
    {
        //BCM_LOG_INFO(BCM_LOG_ID_BPM, "skb is not pristine");
        bpm_pg->skb_pristine_chk_fail_cnt++;
        _bpm_pristine_skb(skb, 1);
    }
#endif   //CC_DBG_BPMSKB_PRISTINE

    /* Attach SKB to buffer */
    /* clearing the shared info region to make sure the attached BPM buffer is pristine */
    _bpm_attach_skb(skb, buf, datalen, false, SKB_CB_ZERO);
    BPM_SKB_POOL_DBG_STATS_ADD(shinfo_reset_cnt, 1);

    return (void *)skb;
}

/*
 * -----------------------------------------------------------------------------
 * function    : _bpm_create_mult_bufs
 * description : BPM buffers are created from kernel by given number of
 * num_bufs. This function has to be called without holding the BPM_LOCK
 * ------------------------------------------------------------------------------
 */
static inline uint32_t _bpm_create_mult_bufs(uint32_t num_bufs, void **pbuf_arry)
{
    BPM_DEFS();
    uint32_t count;
    uint8_t *data_p = NULL; /* Start of "data" buffer. Skip FkBuff, headroom */

    if (pbuf_arry == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_BPM, "pbuf_arry is NULL");
        return 0;
    }
    for (count = 0; count < num_bufs; ) {
        if ((data_p = _bpm_construct_pdata(bpm_pg, GFP_KERNEL)) == NULL) {
            BCM_LOG_ERROR(BCM_LOG_ID_BPM, "Kmem_cache_alloc %d failed",
                          (int)num_bufs);
            break;
        }

        pbuf_arry[count++] = (void *)data_p;
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "Expanded by %4u %8s", count, "RxBufs");
    return count;
}

/*
 * function    : bpm_add_buf_to_pool
 * description : Function expands the buf_pool with an array of BPM buffer
 * pointers. This function has to be called with holding the BPM_LOCK
 * */
static inline uint32_t _bpm_add_buf_to_pool(uint32_t num_bufs, void **pbuf_arry)
{
    BPM_DEFS();
    uint32_t count = 0;
    uint32_t idx;
    void *ptr;
    uint32_t buf_idx;
    count = BPM_BUF_EXPANSION_ROOM(bpm_pg);
    if (count >= num_bufs)
        count = num_bufs;
    else
        BCM_LOG_NOTICE(BCM_LOG_ID_BPM,
                          "More buffers(%u) are created that can be stored(%u) in the bpm pool",
                          num_bufs, count);

    for (idx = 0; idx < count; idx++)
    {
        ptr = _get_buf_md_and_idx(pbuf_arry[idx], &buf_idx);
        _add_buf_to_exp_pool(pbuf_arry[idx], ptr, buf_idx);
    }

    /* Mark the rest of the buffers metadata as invalid ones as these will be
     * freed back */
    for (idx = count; idx < num_bufs; idx++)
    {
        ptr = _get_buf_md_and_idx(pbuf_arry[idx], &buf_idx);
        _bpm_set_buf_state_invalid(pbuf_arry[idx], ptr, buf_idx);
    }

    if (BPM_WM_GET(max_buf_wm) < (BPM_BUF_POOL_TOP_IDX + BPM_BUF_POOL_EXP_TOP_IDX))
        BPM_WM_SET(max_buf_wm,(BPM_BUF_POOL_TOP_IDX + BPM_BUF_POOL_EXP_TOP_IDX));
    if (likely(idx))
        BPM_STATS_INCR(buf_expand_cnt);
    return count;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_grow_skb_pool
 * Description: Extend the skb_pool with N buffers, or upto "bound".
 * This function is not reentrant safe.
 *------------------------------------------------------------------------------
 */
static int bpm_grow_skb_pool(uint32_t skb_grow, gfp_t flags)
{
    uint32_t skb_alloced;
    struct sk_buff *skb, *skb_head, *skb_tail;
    BPM_DEFS();
    BPM_FLAGS_DEF();

    BCM_LOG_FUNC(BCM_LOG_ID_BPM);

    BPM_LOCK_IRQ();

    /* Ensure pool does not get extended beyond configured bound */
    if ((BPM_SKB_POOL_TOTAL + skb_grow) > BPM_SKB_POOL_BOUND) {
        /* permissible extends */
        skb_grow = BPM_SKB_POOL_BOUND - BPM_SKB_POOL_TOTAL;

        if (skb_grow == 0U) { /* no more extensions, as reached bound */
            BPM_UNLOCK_IRQ();
            BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "extend bound failure");
            BPM_STATS_INCR(skb_fail_cnt);
            return BPM_ERROR;
        }
    }

    BPM_UNLOCK_IRQ();

    skb_alloced = 0U;
    skb_head = skb_tail = (struct sk_buff *)NULL;

    do { /* Allocate sk_buffs and place them into local list */
        skb = _bpm_construct_skb(flags); /* allocate sk_buff structure */

        if (!skb) {
            BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "extend skb failure");
            BPM_STATS_INCR(skb_fail_cnt);
            break; /* skbs_alloced may be 0 */
        }

        /* All sk_buffs in BPM pool are in a "pristine" state */
        _bpm_pristine_skb(skb, 1); /* need_reset = 1 */
        BPM_SKB_POOL_DBG_STATS_ADD(skb_hdr_reset_cnt, 1);

        if (skb_alloced) {
            skb->next = (struct sk_buff *)MARK_SKB_IN_BPM_SKBPOOL(skb_head); /* uses skb::next for linking */
            skb_head = skb;
        } else {
            skb_head = skb;
            skb_tail = skb;
        }

    } while (++skb_alloced < skb_grow);

    if (skb_alloced) { /* Add local pool to BPM pool, with lock */
        BPM_LOCK_IRQ();
        _bpm_put_skblist_to_freelist(skb_head, skb_tail, skb_alloced);
        BPM_SKB_POOL_TOTAL += skb_alloced;
        if (BPM_SKB_POOL_TOTAL > BPM_WM_GET(max_skb_wm))
            BPM_WM_SET(max_skb_wm, BPM_SKB_POOL_TOTAL);
		BPM_STATS_INCR(skb_grow_cnt);
        BPM_UNLOCK_IRQ();
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_BPM,
        "skbs pool extended by %u total %u", skb_alloced, BPM_SKB_POOL_TOTAL);

    return skb_alloced;
}   /* bpm_grow_skb_pool() */

static void bpm_buf_metadata_dump(const char *func, int line, void *buf_p)
{
#if defined(CC_DBG_BPM_METADATA_DUMP)
    bpm_buf_metadata *md_ptr = (bpm_buf_metadata *)((uint8_t *)buf_p + BPM_BUF_TO_METADATA_OFFSET);

    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "** %s:%d buf_p %p\n", func, line, buf_p);
#if defined(CC_DBG_BPM_METADATA_INFO_CORRUPTION)
    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "** md_ptr->signature_start = %x\n", md_ptr->signature_start);
    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "** md_ptr->mark = %x\n", md_ptr->mark);
    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "** md_ptr->buffer_state = %x\n", md_ptr->buffer_state);
    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "** md_ptr->signature_end = %x\n", md_ptr->signature_end);
#else
    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "** md_ptr->mark = %x\n", md_ptr->mark);
    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "** md_ptr->buffer_state = %x\n", md_ptr->buffer_state);
#endif
#endif
}

static void bpm_dump_buf_status_counters(void)
{
    BPM_DEFS();
    printk("\n----------------------- BPM Status --------------------------\n");
    printk("base pool tot_buf       = %u\n", BPM_BUF_POOL_TOP_IDX);
    printk("cur                     = %u\n", BPM_BUF_POOL_CUR_IDX);
    printk("exp pool tot_buf        = %u\n", BPM_BUF_POOL_EXP_TOP_IDX);
    printk("exp pool cur            = %u\n", BPM_BUF_POOL_EXP_CUR_IDX);
    printk("avail                   = %u\n", BPM_BUF_AVAIL_CNT(bpm_pg));
    printk("no_buf_err              = %u\n", BPM_STATS_GET(buf_fail_cnt));
    printk("cum_alloc               = %u\n", BPM_STATS_GET(buf_alloc_cnt));
    printk("cum_free                = %u\n", BPM_STATS_GET(buf_free_cnt));
    printk("exp cum_alloc           = %u\n", BPM_STATS_GET(exp_buf_alloc_cnt));
    printk("exp cum_free            = %u\n", BPM_STATS_GET(exp_buf_free_cnt));
    printk("min_availability        = %u\n", BPM_STATS_GET(min_avail_cnt));
    printk("max_inuse_watermark     = %u\n", BPM_WM_GET(max_inuse_buf_wm));
    if (IS_BPM_DYN_ALLOC_MODE()) {
        printk("max_watermark           = %u\n", BPM_WM_GET(max_buf_wm));
        printk("buf_expansion_count     = %u\n", BPM_STATS_GET(buf_expand_cnt));
        printk("buf_shrink_count        = %u\n", BPM_STATS_GET(buf_shrink_cnt));
        printk("buf_exp_fail_count      = %u\n", BPM_STATS_GET(buf_exp_fail_cnt));
        printk("buf_inthr_alloc_count   = %u\n", BPM_STATS_GET(buf_inthr_alloc_cnt));
        printk("skb_inthr_alloc_count   = %u\n", BPM_STATS_GET(skb_inthr_alloc_cnt));
    }
#if defined(CC_DBG_BPMBUF_PRISTINE)
    printk("pristine_chk_fail_cnt   = %u\n", BPM_STATS_GET(buf_pristine_chk_fail_cnt));
#endif
    printk("max_dyn                 = %u\n", bpm_pg->max_dyn);

}

/*
 * -------------------------------------------------------------------------------
 * function    : bpm_buf_expand
 * description : Expands BPM allocated buffer from kernel by given number of
 * num_bufs. This function has to be called by holding the BPM_LOCK
 * ------------------------------------------------------------------------------
 */
static int bpm_buf_expand(uint32_t num_bufs)
{
    BPM_DEFS();
    uint32_t count = 0;
    uint32_t idx;
    void *ptr;
    uint32_t buf_idx;
    uint8_t *data_p = NULL; /* Start of "data" buffer. Skip FkBuff, headroom */
    count = BPM_BUF_EXPANSION_ROOM(bpm_pg);
    if (count >= num_bufs)
        count = num_bufs;
    else
        BCM_LOG_NOTICE(BCM_LOG_ID_BPM,
                          "More buffers(%u) are created that can be stored(%u) in the bpm pool",
                          num_bufs, count);

    for (idx = 0; idx < count; idx++) {
        /* This expansion happens in thread, thus using GFP_ATOMIC */
        if ((data_p = _bpm_construct_pdata(bpm_pg, GFP_ATOMIC)) == NULL) {
            BCM_LOG_ERROR(BCM_LOG_ID_BPM, "Kmem_cache_alloc %d failed",
                          (int)num_bufs);
            break;
        }

        ptr = _get_buf_md_and_idx(data_p, &buf_idx);
        _add_buf_to_exp_pool(data_p, ptr, buf_idx);
    }

    if (BPM_WM_GET(max_buf_wm) < (BPM_BUF_POOL_TOP_IDX + BPM_BUF_POOL_EXP_TOP_IDX))
        BPM_WM_SET(max_buf_wm,(BPM_BUF_POOL_TOP_IDX + BPM_BUF_POOL_EXP_TOP_IDX));
    if (likely(count))
        BPM_STATS_INCR(buf_expand_cnt);
    if (unlikely(count < num_bufs))
    {
        /* Could not allocate requested number of buffers */
        if (BPM_PRINT_FREQ(BPM_STATS_GET(buf_exp_fail_cnt)))
        {
            BCM_LOG_NOTICE(BCM_LOG_ID_BPM,
                          "Could not allocate requested buffers (%u:%u)",
                          count, BPM_BUF_POOL_TOP_IDX);
        }
        BPM_STATS_INCR(buf_exp_fail_cnt);
    }
    BCM_LOG_DEBUG(BCM_LOG_ID_BPM, "Expanded by %4u %8s", count, "RxBufs");
    return count;
}

#endif	//__BPM_CACHE_INLINE_H_INCLUDED__
