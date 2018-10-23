#ifndef _MYCT_H_
#define _MYCT_H_

#include "linux_list.h"

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

struct nf_conntrack;

enum {
	MYCT_NONE		= 0,
	MYCT_ESTABLISHED	= (1 << 0),
};

enum {
	MYCT_DIR_ORIG = 0,
	MYCT_DIR_REPL,
	MYCT_DIR_MAX,
};

union myct_proto {
	uint16_t port;
	uint16_t all;
};

struct myct_man {
	union nfct_attr_grp_addr u3;
	union myct_proto	u;
	uint16_t		l3num;
	uint8_t			protonum;
};

struct myct_tuple {
	struct myct_man		src;
	struct myct_man		dst;
};

struct myct {
	struct nf_conntrack *ct;
	struct nf_expect *exp;
	void *priv_data;
};

#endif
