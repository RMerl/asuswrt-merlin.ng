#ifndef _SHA256_H
#define _SHA256_H

#define SHA256_SUM_LEN	32
#define SHA256_DER_LEN	19

extern const uint8_t sha256_der_prefix[];

/* Reset watchdog each time we process this many bytes */
#define CHUNKSZ_SHA256	(64 * 1024)

typedef struct {
	uint32_t total[2];
	uint32_t state[8];
	uint8_t buffer[64];
} sha256_context;

void sha256_starts(sha256_context * ctx);
void sha256_update(sha256_context *ctx, const uint8_t *input, uint32_t length);
void sha256_finish(sha256_context * ctx, uint8_t digest[SHA256_SUM_LEN]);

void sha256_csum_wd(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);

#endif /* _SHA256_H */
