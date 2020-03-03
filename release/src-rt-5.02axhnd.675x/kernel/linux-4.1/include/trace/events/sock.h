#undef TRACE_SYSTEM
#define TRACE_SYSTEM sock

#if !defined(_TRACE_SOCK_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_SOCK_H

#include <net/sock.h>
#include <linux/tracepoint.h>

TRACE_EVENT(sock_rcvqueue_full,

	TP_PROTO(struct sock *sk, struct sk_buff *skb),

	TP_ARGS(sk, skb),

	TP_STRUCT__entry(
		__field(int, rmem_alloc)
		__field(unsigned int, truesize)
		__field(int, sk_rcvbuf)
	),

	TP_fast_assign(
		__entry->rmem_alloc = atomic_read(&sk->sk_rmem_alloc);
		__entry->truesize   = skb->truesize;
		__entry->sk_rcvbuf  = sk->sk_rcvbuf;
	),

	TP_printk("rmem_alloc=%d truesize=%u sk_rcvbuf=%d",
		__entry->rmem_alloc, __entry->truesize, __entry->sk_rcvbuf)
);

TRACE_EVENT(sock_exceed_buf_limit,

	TP_PROTO(struct sock *sk, struct proto *prot, long allocated),

	TP_ARGS(sk, prot, allocated),

	TP_STRUCT__entry(
		__array(char, name, 32)
		__field(long *, sysctl_mem)
		__field(long, allocated)
		__field(int, sysctl_rmem)
		__field(int, rmem_alloc)
	),

	TP_fast_assign(
		strncpy(__entry->name, prot->name, 32);
		__entry->sysctl_mem = prot->sysctl_mem;
		__entry->allocated = allocated;
		__entry->sysctl_rmem = prot->sysctl_rmem[0];
		__entry->rmem_alloc = atomic_read(&sk->sk_rmem_alloc);
	),

	TP_printk("proto:%s sysctl_mem=%ld,%ld,%ld allocated=%ld "
		"sysctl_rmem=%d rmem_alloc=%d",
		__entry->name,
		__entry->sysctl_mem[0],
		__entry->sysctl_mem[1],
		__entry->sysctl_mem[2],
		__entry->allocated,
		__entry->sysctl_rmem,
		__entry->rmem_alloc)
);

#endif /* _TRACE_SOCK_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
