#ifndef _LIBNFQUEUE_UDP_H_
#define _LIBNFQUEUE_UDP_H_

struct pkt_buff;

struct udphdr *nfq_udp_get_hdr(struct pkt_buff *pktb);
void *nfq_udp_get_payload(struct udphdr *udph, struct pkt_buff *pktb);
unsigned int nfq_udp_get_payload_len(struct udphdr *udph, struct pkt_buff *pktb);

void nfq_udp_compute_checksum_ipv4(struct udphdr *udph, struct iphdr *iph);
void nfq_udp_compute_checksum_ipv6(struct udphdr *udph, struct ip6_hdr *ip6h);

int nfq_udp_mangle_ipv4(struct pkt_buff *pktb, unsigned int match_offset, unsigned int match_len, const char *rep_buffer, unsigned int rep_len);
int nfq_udp_mangle_ipv6(struct pkt_buff *pktb, unsigned int match_offset, unsigned int match_len, const char *rep_buffer, unsigned int rep_len);

int nfq_udp_snprintf(char *buf, size_t size, const struct udphdr *udp);

#endif
