#ifndef _PKTBUFF_H_
#define _PKTBUFF_H_

struct pkt_buff;

struct pkt_buff *pktb_alloc(int family, void *data, size_t len, size_t extra);
void pktb_free(struct pkt_buff *pktb);

uint8_t *pktb_data(struct pkt_buff *pktb);
uint32_t pktb_len(struct pkt_buff *pktb);

void pktb_push(struct pkt_buff *pktb, unsigned int len);
void pktb_pull(struct pkt_buff *pktb, unsigned int len);
void pktb_put(struct pkt_buff *pktb, unsigned int len);
void pktb_trim(struct pkt_buff *pktb, unsigned int len);
unsigned int pktb_tailroom(struct pkt_buff *pktb);

uint8_t *pktb_mac_header(struct pkt_buff *pktb);
uint8_t *pktb_network_header(struct pkt_buff *pktb);
uint8_t *pktb_transport_header(struct pkt_buff *pktb);

int pktb_mangle(struct pkt_buff *pkt, unsigned int dataoff, unsigned int match_offset, unsigned int match_len, const char *rep_buffer, unsigned int rep_len);

bool pktb_mangled(const struct pkt_buff *pktb);

#endif
