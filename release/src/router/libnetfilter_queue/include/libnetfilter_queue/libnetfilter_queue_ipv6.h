#ifndef _LIBNFQUEUE_H_
#define _LIBNFQUEUE_H_

struct pkt_buff;
struct ip6_hdr;

struct ip6_hdr *nfq_ip6_get_hdr(struct pkt_buff *pktb);
int nfq_ip6_set_transport_header(struct pkt_buff *pktb, struct ip6_hdr *iph, uint8_t target);
int nfq_ip6_mangle(struct pkt_buff *pktb, unsigned int dataoff,unsigned int match_offset, unsigned int match_len,const char *rep_buffer, unsigned int rep_len);
int nfq_ip6_snprintf(char *buf, size_t size, const struct ip6_hdr *ip6h);

#endif
