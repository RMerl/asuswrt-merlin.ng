#ifndef _ORIGIN_H_
#define _ORIGIN_H_

enum {
	CTD_ORIGIN_NOT_ME = 0,		/* this event comes from the kernel or
					   any process, but not conntrackd */
	CTD_ORIGIN_COMMIT,		/* event comes from committer */
	CTD_ORIGIN_FLUSH,		/* event comes from flush */
	CTD_ORIGIN_INJECT,		/* event comes from direct inject */
};

int origin_register(struct nfct_handle *h, int origin_type);
int origin_find(const struct nlmsghdr *nlh);
int origin_unregister(struct nfct_handle *h);

#endif
