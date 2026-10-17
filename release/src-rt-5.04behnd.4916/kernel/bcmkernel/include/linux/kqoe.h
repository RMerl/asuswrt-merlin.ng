#ifndef _KQOE_H
#define _KQOE_H
#include <uapi/linux/kqoe.h>

struct kqoe_ct_info {
	s16		orig_queue;
	s16		reply_queue;
	u8		eg_prio;
	u8		dscp;
	u8		in_tid;
	u8		eg_tid;
	unsigned long	flags;
};

#endif /* _KQOE_H */
