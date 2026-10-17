#ifndef _SHA512_H
#define _SHA512_H

#define SHA384_SUM_LEN          48
#define SHA384_DER_LEN          19
#define SHA512_SUM_LEN          64
#define SHA512_DER_LEN          19
#define SHA512_BLOCK_SIZE       128

#define CHUNKSZ_SHA384	(16 * 1024)
#define CHUNKSZ_SHA512	(16 * 1024)

typedef struct {
	uint64_t state[SHA512_SUM_LEN / 8];
	uint64_t count[2];
	uint8_t buf[SHA512_BLOCK_SIZE];
} sha512_context;

extern const uint8_t sha512_der_prefix[];

void sha512_starts(sha512_context * ctx);
void sha512_update(sha512_context *ctx, const uint8_t *input, uint32_t length);
void sha512_finish(sha512_context * ctx, uint8_t digest[SHA512_SUM_LEN]);

void sha512_csum_wd(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);

extern const uint8_t sha384_der_prefix[];

void sha384_starts(sha512_context * ctx);
void sha384_update(sha512_context *ctx, const uint8_t *input, uint32_t length);
void sha384_finish(sha512_context * ctx, uint8_t digest[SHA384_SUM_LEN]);

void sha384_csum_wd(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);


#endif /* _SHA512_H */
