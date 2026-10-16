#ifndef _XT_NFACCT_MATCH_H
#define _XT_NFACCT_MATCH_H

#include <linux/types.h>

#ifndef NFACCT_NAME_MAX
#define NFACCT_NAME_MAX 32
#endif

struct nf_acct;

struct xt_nfacct_match_info {
	char		name[NFACCT_NAME_MAX];
	struct nf_acct	*nfacct;
};

struct xt_nfacct_match_info_v1 {
	char		name[NFACCT_NAME_MAX];
	struct nf_acct	*nfacct __attribute__((aligned(8)));
};

#endif /* _XT_NFACCT_MATCH_H */
