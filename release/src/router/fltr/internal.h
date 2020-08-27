/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef __FW_INTERNAL_H__
#define __FW_INTERNAL_H__

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31))
#define NF_INET_LOCAL_OUT	NF_IP_LOCAL_OUT
#define NF_INET_LOCAL_IN	NF_IP_LOCAL_IN
#define NF_INET_FORWARD		NF_IP_FORWARD
#define NF_INET_PRE_ROUTING	NF_IP_PRE_ROUTING
#define NF_INET_POST_ROUTING	NF_IP_POST_ROUTING
#endif

#ifndef pr_emerg
#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif
#define pr_emerg(fmt, ...) \
		printk(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#endif

#define BW_ERR          -1
#define BW_OK           0
#define BW_QUEUE        1

typedef struct {
	atomic_t null_skb_cnt;
	atomic_t null_skb_dev_cnt;
	atomic_t null_skb_ct_cnt;

	atomic_t nonip_cnt;
	
	atomic_t drop_cnt;
	atomic_t stolen_cnt;

	atomic_t fastpath_cnt;
	atomic_t fastpath_conn_cnt;

	atomic_t unexp_cnt;
	atomic_t unmatched_sip_cnt;
	atomic_t unmatched_dip_cnt;
	atomic_t unmatched_sport_cnt;
	atomic_t unmatched_dport_cnt;
} flt_stat_t;

#define CNT_INC(__c)		atomic_inc(&flt_stat.__c ## _cnt)
#define CNT_READ(__c)		atomic_read(&flt_stat.__c ## _cnt)

extern flt_stat_t flt_stat;

extern volatile bool rmmod_in_progress;
extern DEFINE_PER_CPU(bool, handle_pkt);

#ifndef __get_cpu_var
#define __get_cpu_var(var) (*this_cpu_ptr(&(var)))
#endif

#define HOOK_PROLOG(__ret)			\
do { 						\
	if (unlikely(rmmod_in_progress))	\
	{ 					\
		return (__ret);			\
	}					\
	__get_cpu_var(handle_pkt) = true;	\
} while(0)

#define HOOK_EPILOG(__ret)			\
do { 						\
	__get_cpu_var(handle_pkt) = false;	\
	return (__ret);				\
} while(0)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#define DEFINE_HOOK_FUNC(__func) \
uint32_t forward_##__func(   \
	const void *priv, struct sk_buff *skb, \
        const struct nf_hook_state *state) \
{          \
         return (__func(state->hook, state->sk, skb, state->in, state->out, state->okfn)); \
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
#define DEFINE_HOOK_FUNC(__func) \
uint32_t forward_##__func(   \
	const struct nf_hook_ops *ops, struct sk_buff *skb, \
	const struct nf_hook_state *state) \
{          \
	 return (__func(ops->hooknum, state->sk, skb, state->in, state->out, state->okfn)); \
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,1))
#define DEFINE_HOOK_FUNC(__func) \
uint32_t forward_##__func(   \
	const struct nf_hook_ops *ops, struct sk_buff *skb, \
	const struct net_device *in_dev, const struct net_device *out_dev, \
	void* okfn) \
{          \
	 return (__func(ops->hooknum, NULL, skb, in_dev, out_dev, okfn)); \
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
#define DEFINE_HOOK_FUNC(__func)	\
uint32_t forward_##__func(			\
	uint32_t hooknum, struct sk_buff *skb,	\
	const struct net_device *in_dev, const struct net_device *out_dev,	\
	void* okfn)	\
{ 									\
	return (__func(hooknum, NULL, skb, in_dev, out_dev, okfn));	\
}
#else
#define DEFINE_HOOK_FUNC(__func)	\
uint32_t forward_##__func(			\
	uint32_t hooknum, struct sk_buff **pskb,	\
	const struct net_device *in_dev, const struct net_device *out_dev,	\
	void* okfn)	\
{ 									\
	return (__func(hooknum, NULL, *pskb, in_dev, out_dev, okfn));	\
}
#endif

#define DEFINE_HOOK(__func)		\
	static uint32_t __func(				\
		uint32_t hooknum, struct sock *sk, struct sk_buff *skb,	\
		const struct net_device *in_dev, const struct net_device *out_dev,	\
		void* okfn);	\
	DEFINE_HOOK_FUNC(__func);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#define MY_NF_HOOK_THRESH(pf, hook, net, sk, skb, in, out, okfn, thresh) \
        NF_HOOK_THRESH(pf, hook, net, sk, skb, in, out, okfn, thresh)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
#define MY_NF_HOOK_THRESH(pf, hook, sk, skb, in, out, okfn, thresh) \
	NF_HOOK_THRESH(pf, hook, sk, skb, in, out, okfn, thresh)
#else
#define MY_NF_HOOK_THRESH(pf, hook, sk, skb, in, out, okfn, thresh) \
	NF_HOOK_THRESH(pf, hook, skb, in, out, okfn, thresh)
#endif

#define CREATE_PROC_SEQ_FILE(ptr, name, fops)		\
do {												\
	ptr = create_proc_entry(name, S_IRUGO, NULL);	\
	if (!ptr)										\
	{												\
		printk("failed to create %s!\n", name);		\
		return -1;									\
	}												\
	else											\
	{												\
		ptr->proc_fops = &fops;						\
		DBG(#fops" hooked\n");						\
	}												\
} while(0)

#endif 	// !__FW_INTERNAL_H__


