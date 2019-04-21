#ifndef _TRAFFIC_STATS_H_
#define _TRAFFIC_STATS_H_

struct nf_conntrack;

void update_traffic_stats(struct nf_conntrack *ct);

void dump_traffic_stats(int fd);

#endif
