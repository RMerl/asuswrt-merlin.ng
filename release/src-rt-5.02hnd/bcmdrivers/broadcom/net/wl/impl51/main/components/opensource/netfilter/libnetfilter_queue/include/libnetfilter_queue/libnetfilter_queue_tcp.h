#ifndef _LIBNFQUEUE_TCP_H_
#define _LIBNFQUEUE_TCP_H_

struct pkt_buff;

struct tcphdr *nfq_tcp_get_hdr(struct pkt_buff *pktb);
void *nfq_tcp_get_payload(struct tcphdr *tcph, struct pkt_buff *pktb);
unsigned int nfq_tcp_get_payload_len(struct tcphdr *tcph, struct pkt_buff *pktb);

struct iphdr;
struct ip6_hdr;

void nfq_tcp_compute_checksum_ipv4(struct tcphdr *tcph, struct iphdr *iph);
void nfq_tcp_compute_checksum_ipv6(struct tcphdr *tcph, struct ip6_hdr *ip6h);

int nfq_tcp_mangle_ipv4(struct pkt_buff *pkt, unsigned int match_offset, unsigned int match_len, const char *rep_buffer, unsigned int rep_len);

int nfq_tcp_snprintf(char *buf, size_t size, const struct tcphdr *tcp);

#endif
