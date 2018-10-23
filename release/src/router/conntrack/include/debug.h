#ifndef _DEBUG_H
#define _DEBUG_H

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#undef DEBUG_CT

#ifdef DEBUG_CT
#define debug_ct(ct, msg)					\
({								\
	char buf[1024];						\
	nfct_snprintf(buf, 1024, ct, NFCT_T_ALL, 0, 0);		\
	printf("[%s]: %s\n", msg, buf);				\
})
#define debug printf
#else
#define debug_ct(ct, msg) do {} while (0)
#define debug(...) do {} while (0)
#endif

#endif
