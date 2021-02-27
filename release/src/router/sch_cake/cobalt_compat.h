#ifndef __NET_SCHED_COBALT_COMPAT_H
#define __NET_SCHED_COBALT_COMPAT_H
/* Backport some stuff if needed.
 */
#if KERNEL_VERSION(3, 11, 0) > LINUX_VERSION_CODE
#define ktime_add_ms(kt, msec) ktime_add_ns(kt, msec * NSEC_PER_MSEC)
#endif

#if KERNEL_VERSION(3, 14, 0) > LINUX_VERSION_CODE

static inline u32 reciprocal_scale(u32 val, u32 ep_ro)
{
	return (u32)(((u64) val * ep_ro) >> 32);
}

#endif

#if KERNEL_VERSION(3, 15, 0) > LINUX_VERSION_CODE

static inline void kvfree(const void *addr)
{
	if (is_vmalloc_addr(addr))
		vfree(addr);
	else
		kfree(addr);
}

#endif

#if KERNEL_VERSION(3, 16, 0) > LINUX_VERSION_CODE
#define ktime_after(cmp1, cmp2) ktime_compare(cmp1, cmp2) > 0
#define ktime_before(cmp1, cmp2) ktime_compare(cmp1, cmp2) < 0
#endif

#if KERNEL_VERSION(3, 17, 0) > LINUX_VERSION_CODE

#define ktime_get_ns() ktime_to_ns(ktime_get())

#endif

/* 3.18 > 4.7 use 3 arg, everything else uses 2 arg versions
 * of qdisc_watchdog_schedule_ns
 */
#if ((KERNEL_VERSION(3, 18, 0) <= LINUX_VERSION_CODE) && (KERNEL_VERSION(4, 8, 0) > LINUX_VERSION_CODE))
#define qdisc_watchdog_schedule_ns(_a, _b) qdisc_watchdog_schedule_ns(_a, _b, true);
#endif

#if KERNEL_VERSION(3, 18, 0) > LINUX_VERSION_CODE
static inline void qdisc_qstats_backlog_dec(struct Qdisc *sch,
					    const struct sk_buff *skb)
{
	sch->qstats.backlog -= qdisc_pkt_len(skb);
}

static inline void qdisc_qstats_backlog_inc(struct Qdisc *sch,
					    const struct sk_buff *skb)
{
	sch->qstats.backlog += qdisc_pkt_len(skb);
}

static inline void __qdisc_qstats_drop(struct Qdisc *sch, int count)
{
	sch->qstats.drops += count;
}

static inline void qdisc_qstats_drop(struct Qdisc *sch)
{
	sch->qstats.drops++;
}

#define gnet_stats_copy_queue(_a, _b, _c, _d) gnet_stats_copy_queue(_a, _c)

#endif

#if KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE
#define TCPOPT_FASTOPEN	34
#endif

#if KERNEL_VERSION(4, 3, 0) > LINUX_VERSION_CODE
#define tcf_classify(_a, _b, _c, _d) tc_classify(_a, _b, _c);
#elif KERNEL_VERSION(4, 13, 0) > LINUX_VERSION_CODE
#define tcf_classify(_a, _b, _c, _d) tc_classify(_a, _b, _c, _d);
#endif

#if !defined(IS_REACHABLE)
#define IS_REACHABLE(option) (config_enabled(option) || \
		(config_enabled(option##_MODULE) && config_enabled(MODULE)))
#endif

#if ((KERNEL_VERSION(4, 4, 114) > LINUX_VERSION_CODE) && \
     ((KERNEL_VERSION(4,  1, 50) > LINUX_VERSION_CODE) || (KERNEL_VERSION(4,  2, 0) <= LINUX_VERSION_CODE)))
static inline unsigned int __tcp_hdrlen(const struct tcphdr *th)
{
	return th->doff * 4;
}
#endif

#if KERNEL_VERSION(4, 6, 0) > LINUX_VERSION_CODE
static inline int skb_try_make_writable(struct sk_buff *skb,
					unsigned int write_len)
{
	return skb_cloned(skb) && !skb_clone_writable(skb, write_len) &&
	       pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
}
#endif

#if KERNEL_VERSION(4, 11, 0) > LINUX_VERSION_CODE
static inline int skb_mac_offset(const struct sk_buff *skb)
{
	return skb_mac_header(skb) - skb->data;
}
#endif

#if KERNEL_VERSION(4, 7, 0) > LINUX_VERSION_CODE
#define nla_put_u64_64bit(skb, attrtype, value, padattr) nla_put_u64(skb, attrtype, value)
#endif

#if KERNEL_VERSION(4, 8, 0) > LINUX_VERSION_CODE
#define cake_maybe_lock(sch)
#define cake_maybe_unlock(sch)
#else
#define cake_maybe_lock(sch) sch_tree_lock(sch);
#define cake_maybe_unlock(sch) sch_tree_unlock(sch);
#endif


#if KERNEL_VERSION(4, 12, 0) > LINUX_VERSION_CODE
static void *kvzalloc(size_t sz, gfp_t flags)
{
	void *ptr = kzalloc(sz, flags);

	if (!ptr)
		ptr = vzalloc(sz);
	return ptr;
}
#endif

/* save the best till last
 * qdisc_tree_reduce_backlog appears in kernel from:
3.16.37 onward
not in 3.17
3.18.37
not in 3.19
not in 4.0
4.1.28 onward
not in 4.2
not in 4.3
4.4.11 onward
4.5.5 onward
 */
#if ((KERNEL_VERSION(3,  0, 0) <= LINUX_VERSION_CODE) && (KERNEL_VERSION(3, 16, 37) > LINUX_VERSION_CODE)) || \
    ((KERNEL_VERSION(3, 18, 0) <= LINUX_VERSION_CODE) && (KERNEL_VERSION(3, 18, 37) > LINUX_VERSION_CODE)) || \
    ((KERNEL_VERSION(4,  1, 0) <= LINUX_VERSION_CODE) && (KERNEL_VERSION(4,  1, 28) > LINUX_VERSION_CODE)) || \
    ((KERNEL_VERSION(4,  4, 0) <= LINUX_VERSION_CODE) && (KERNEL_VERSION(4,  4, 11) > LINUX_VERSION_CODE)) || \
    ((KERNEL_VERSION(4,  5, 0) <= LINUX_VERSION_CODE) && (KERNEL_VERSION(4,  5,  5) > LINUX_VERSION_CODE))
#define qdisc_tree_reduce_backlog(_a, _b, _c) qdisc_tree_decrease_qlen(_a, _b)
#endif


#endif
