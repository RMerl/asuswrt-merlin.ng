#ifndef _XT_BLOG_H
#define _XT_BLOG_H

#include <linux/types.h>

struct xt_blog {
	__u8	tcp_pure_ack;
	__u8	invert;
};

#endif /*_XT_BLOG_H*/
