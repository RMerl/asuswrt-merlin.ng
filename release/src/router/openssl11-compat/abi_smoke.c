#define SSL_CTX_set_options SSL_CTX_set_options__openssl3_decl
#include <stdio.h>
#include <stdlib.h>

#include <openssl/ssl.h>
#undef SSL_CTX_set_options

extern unsigned long SSL_CTX_set_options(SSL_CTX *ctx, unsigned long op);

int main(void)
{
	SSL_CTX *ctx;
	unsigned long mask;
	unsigned long version;

	ctx = SSL_CTX_new(TLS_client_method());
	if (ctx == NULL) {
		fprintf(stderr, "SSL_CTX_new failed\n");
		return 1;
	}

	mask = SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;
	if ((SSL_CTX_set_options(ctx, mask) & mask) != mask) {
		fprintf(stderr, "SSL_CTX_set_options ABI check failed\n");
		SSL_CTX_free(ctx);
		return 1;
	}

	version = OpenSSL_version_num();
	if ((version >> 28) != 1) {
		fprintf(stderr, "OpenSSL_version_num compatibility check failed: 0x%08lx\n", version);
		SSL_CTX_free(ctx);
		return 1;
	}

	SSL_CTX_free(ctx);
	return 0;
}
