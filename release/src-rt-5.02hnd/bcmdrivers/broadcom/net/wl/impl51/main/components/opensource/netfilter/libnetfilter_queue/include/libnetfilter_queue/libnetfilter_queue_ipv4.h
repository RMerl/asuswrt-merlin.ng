#ifndef _LIBNFQUEUE_IPV4_
#define _LIBNFQUEUE_IPV4_

struct pkt_buff;
struct iphdr;

struct iphdr *nfq_ip_get_hdr(struct pkt_buff *pktb);
int nfq_ip_set_transport_header(struct pkt_buff *pktb, struct iphdr *iph);
void nfq_ip_set_checksum(struct iphdr *iph);
int nfq_ip_mangle(struct pkt_buff *pkt, unsigned int dataoff, unsigned int match_offset, unsigned int match_len, const char *rep_buffer, unsigned int rep_len);
int nfq_ip_snprintf(char *buf, size_t size, const struct iphdr *iph);

#endif
