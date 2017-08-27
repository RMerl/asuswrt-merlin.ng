#ifndef __LL_MAP_H__
#define __LL_MAP_H__ 1

int ll_remember_index(const struct sockaddr_nl *who,
		      struct nlmsghdr *n, void *arg);

void ll_init_map(struct rtnl_handle *rth);
unsigned ll_name_to_index(const char *name);
const char *ll_index_to_name(unsigned idx);
const char *ll_idx_n2a(unsigned idx, char *buf);
int ll_index_to_type(unsigned idx);
int ll_index_to_flags(unsigned idx);
unsigned namehash(const char *str);

#endif /* __LL_MAP_H__ */
