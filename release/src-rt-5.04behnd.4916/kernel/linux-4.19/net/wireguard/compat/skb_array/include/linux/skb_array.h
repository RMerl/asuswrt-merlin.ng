#ifndef _WG_SKB_ARRAY_H
#define _WG_SKB_ARRAY_H

#include <linux/skbuff.h>

static void __skb_array_destroy_skb(void *ptr)
{
	kfree_skb(ptr);
}

#endif
