#include <stdint.h>

#define PKT_SIZE 2820

struct image_pkt_hdr {
	uint32_t resend;
	uint32_t totcrc;
	uint32_t nr_blocks;
	uint32_t blocksize;
	uint32_t block_crc;
	uint32_t block_nr;
	uint32_t pkt_sequence;
	uint16_t pkt_nr;
	uint16_t nr_pkts;
	uint32_t thislen;
	uint32_t thiscrc;
};

struct image_pkt {
	struct image_pkt_hdr hdr;
	unsigned char data[PKT_SIZE];
};
