#ifndef __NFCT_H__
#define __NFCT_H__

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include "list.h"
#include "nfcm.h"

extern int nf_process_conntrack(const struct nf_conntrack *ct,
								struct list_head *iplist,
                                struct list_head *list);
extern nf_node_t *nf_node_new();
extern void nf_node_free(nf_node_t *nn);
extern bool find_dot_in_ip_str(char *ipstr);

#endif // __NFCT_H__
