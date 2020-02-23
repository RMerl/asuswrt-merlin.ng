#if defined(CONFIG_BCM_KF_NETFILTER) && defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#ifndef __LINUX_BRIDGE_EBT_BLOG_H
#define __LINUX_BRIDGE_EBT_BLOG_H

#include <linux/types.h>

struct ebt_blog_info
{
	__u8	tcp_pure_ack;
	__u8	invert;
};

#endif
#endif
